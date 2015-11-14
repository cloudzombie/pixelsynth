#include "widget.h"
#include "model.h"
#include "../property_editors/delegate.h"

#include <core/utils.h>

using Editor::Modules::Timeline::Widget;

using namespace Core;

Widget::Widget(QWidget* parent, Project& project)
	: QDockWidget(parent)
	, project_(project)
	, tree_(new QTreeView(this))
	, model_(std::make_shared<Model>())
{
	setWindowTitle(tr("Timeline"));
	setWidget(tree_);

	proxy_ = new QSortFilterProxyModel(this);
	proxy_->setSourceModel(model_.get());
	tree_->setModel(proxy_);
	tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
	tree_->setItemDelegateForColumn(1, new PropertyEditors::Delegate(tree_));

	connect(model_.get(), &Model::propertyChanged, this, [&](const Property* prop, PropertyValue value)
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
}

void Widget::projectMutated(std::shared_ptr<MutationInfo> mutationInfo) const
{
	auto selection = proxy_->mapSelectionToSource(tree_->selectionModel()->selection());
	auto newSelection = model_->apply(mutationInfo, selection.indexes());
	tree_->selectionModel()->clear();
	for (auto&& item : newSelection) tree_->selectionModel()->select(proxy_->mapFromSource(item), QItemSelectionModel::Select);
}

void Widget::mutate()
{
	auto&& p = project_;
	switch (mutationIndex++)
	{
	case 0:
		p.mutate([&](auto& mut)
		{
			mut.append({ makeNode(hash("DummyNode"), "a") });
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