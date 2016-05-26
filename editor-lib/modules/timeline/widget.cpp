#include "widget.h"
#include "model.h"
#include "proxy_model.h"
#include "keyframer/tree_view.h"
#include "../property_editors/delegate.h"
#include "../../event_bus.h"

#include <core/utils.h>
#include <core/mutation_info.h>

using Editor::Modules::Timeline::Widget;

using namespace Core;

Widget::Widget(QWidget* parent, Project& project)
	: QDockWidget(parent)
	, project_(project)
	, container_(new QWidget(this))
	, layout_(new QGridLayout())
	, tree_(new QTreeView(this))
	, splitter_(new QSplitter(this))
	, proxy_(new ProxyModel(this))
	, model_(std::make_shared<Model>())
{
	setWindowTitle(tr("Timeline"));

	proxy_->setSourceModel(model_.get());

	keyframer_ = new Keyframer::TreeView(project, *proxy_, *model_, this);

	splitter_->addWidget(tree_);
	splitter_->addWidget(keyframer_);
	splitter_->setStretchFactor(1, 1);

	layout_->addWidget(splitter_, 0, 0, 1, 2);
	container_->setLayout(layout_);
	setWidget(container_);

	tree_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	keyframer_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	tree_->setModel(proxy_);
	tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
	tree_->setItemDelegateForColumn(static_cast<int>(Model::Columns::Value), new PropertyEditors::Delegate(tree_));

	connect(tree_, &QTreeView::expanded, keyframer_, &QTreeView::expand);
	connect(tree_, &QTreeView::collapsed, keyframer_, &QTreeView::collapse);
	connect(tree_->verticalScrollBar(), &QScrollBar::valueChanged, this, &Widget::syncVerticalScrollBars);
	connect(keyframer_->verticalScrollBar(), &QScrollBar::valueChanged, this, &Widget::syncVerticalScrollBars);

	connect(&EventBus::instance(), &EventBus::propertyChanged, this, [&](const Property* prop, PropertyValue value)
	{
		project.mutate([&](Document::Builder& mut)
		{
			mut.mutate(project.current().parent(*prop), [&](Node::Builder& node)
			{
				node.mutateProperty(hash(prop->metadata().title().c_str()), [&](Property::Builder& p)
				{
					p.set(0, value);
				});
			});
		}, "edit " + prop->metadata().title());
	});

	// Whenever the selection changes, emit this to the global event bus
	connect(tree_->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& selected, const QItemSelection& deselected) {
		static auto setSelected = [this](const QItemSelectionRange& range, bool selected)
		{
			for (auto& index : range.indexes())
			{
				if (index.column() != 0) continue; // all columns in a row are identical for our purposes, so ignore
				auto node = model_->nodeFromIndex(proxy_->mapToSource(index));
				if (node) emit EventBus::instance().nodeSelectionChanged(node, selected);
			}
		};

		for (auto& range : selected) setSelected(range, true);
		for (auto& range : deselected) setSelected(range, false);
	});

	// Whenever global selection changes, apply it to the tree selection model
	connect(&EventBus::instance(), &EventBus::nodeSelectionChanged, this, [this](NodePtr node, bool selected) {
		QSignalBlocker blocker(&EventBus::instance()); // since this event came from the event bus already, we don't need to trigger the same event again based on changes in the tree selectionmodel
		tree_->selectionModel()->select(proxy_->mapFromSource(model_->findItemIndex(node)), (selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect) | QItemSelectionModel::Rows);
	});
}

void Widget::projectMutated(std::shared_ptr<MutationInfo> mutationInfo) const
{
	auto selection = proxy_->mapSelectionToSource(tree_->selectionModel()->selection()).indexes();
	auto expanded = keyframer_->expanded();
	auto mutatedIndices = model_->apply(mutationInfo);

	// Hack to make sure we only see the item column in the keyframer
	keyframer_->setColumnHidden(static_cast<int>(Model::Columns::Value), true);

	// Update and create item widgets
	updateItemWidgets(model_->invisibleRootItem());

	// Make sure any new rows have the same selection and expansion
	auto i = mutatedIndices.constBegin();
	while (i != mutatedIndices.constEnd())
	{
		if (selection.contains(i.key()))
		{
			tree_->selectionModel()->select(proxy_->mapFromSource(i.value()), QItemSelectionModel::Select | QItemSelectionModel::Rows);
		}

		if (expanded.contains(i.key()))
		{
			tree_->expand(proxy_->mapFromSource(i.value()));
		}

		++i;
	}
}

void Widget::updateItemWidgets(QStandardItem* parent) const
{
	for (auto t = 0; t < parent->rowCount();t++)
	{
		auto child = parent->child(t);
		if (child->hasChildren()) updateItemWidgets(child);
		
		auto keyframeChild = parent->child(t, static_cast<int>(Model::Columns::Item));
		keyframer_->openPersistentEditor(proxy_->mapFromSource(keyframeChild->index()));
	}
}

void Widget::syncVerticalScrollBars(int value) const
{
	if (tree_->verticalScrollBar()->value() != value) tree_->verticalScrollBar()->setValue(value);
	if (keyframer_->verticalScrollBar()->value() != value) keyframer_->verticalScrollBar()->setValue(value);
}

void Widget::deleteSelected() const
{
	keyframer_->deleteSelected();
}

void Widget::mutate()
{
	auto&& p = project_;
	switch (mutationIndex++)
	{
	case 0:
		p.mutate({
			[&](Document::Builder& mut) {
				mut.append({ makeNode(hash("DummyNode"), "a") });
			},
			[&](Document::Builder& mut) {
				mut.mutate(findNode(p, "a"), [&](Node::Builder& node) {
					node.mutateProperty(hash("Vec3"), [&](Property::Builder& prop) {
						prop.set(0, glm::vec3(1, 2, 3));
						prop.set(100, glm::vec3(100, 200, 300));
					});
				});
			}
		}, "create node");
		break;
	case 1:
		p.mutate({
			[&](auto& mut) { mut.insertBefore(findNode(p, "a"), { makeNode(hash("DummyNode"), "b") }); },
			[&](auto& mut) { mut.insertBefore(findNode(p, "b"), { makeNode(hash("DummyNode"), "c") }); }
		}, "create nodes");
		break;
	case 2:
		p.mutate([&](auto& mut) { mut.insertBefore(findNode(p, "c"), { makeNode(hash("DummyNode"), "d") }); });
		break;
	case 3:
		p.mutate([&](auto& mut) { mut.insertBefore(findNode(p, "d"), { makeNode(hash("DummyNode"), "e") }); });
		break;
	case 4:
		p.mutate([&](auto& mut) { mut.erase({ findNode(p, "b"), findNode(p, "d") }); });
		break;
	case 5:
		p.undo();
		break;
	case 6:
		p.mutate([&](Document::Builder& mut)
		{
			auto node_a = findNode(p, "a");
			auto node_b = findNode(p, "b");
			auto node_c = findNode(p, "c");
			mut.reparent(node_c, { node_a, node_b });
		});
		break;
	case 7:
		p.undo();
		break;
	case 8:
		p.mutate([&](Document::Builder& mut)
		{
			mut.mutate(findNode(p, "a"), [&](Node::Builder& node)
			{
				node.mutateProperty(hash("$Title"), [&](Property::Builder& prop) { prop.set(0, "a_new"); });
			});
		});
		break;
	case 9:
		p.undo();
		break;

	default:
		break;
	}
}