#include "widget.h"
#include "model.h"
#include "../property_editors/delegate.h"

#include <core/utils.h>
#include <editor-lib/application.h>

using Editor::Modules::Timeline::Widget;

using namespace Core;

Widget::Widget(QWidget* parent)
	: QDockWidget(parent)
	, tree_(new QTreeView(this))
	, model_(std::make_shared<Model>())
{
	setWindowTitle(tr("Timeline"));
	setWidget(tree_);

	auto proxy = new QSortFilterProxyModel(this);
	proxy->setSourceModel(model_.get());
	tree_->setModel(proxy);
	tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
	tree_->setItemDelegateForColumn(1, new PropertyEditors::Delegate(tree_));

	connect(static_cast<Application*>(qApp), &Application::projectMutated, this, [this, proxy](auto mutationInfo)
	{
		auto selection = proxy->mapSelectionToSource(tree_->selectionModel()->selection());
		auto newSelection = model_->apply(mutationInfo, selection.indexes());
		tree_->selectionModel()->clear();
		for (auto&& item: newSelection) tree_->selectionModel()->select(proxy->mapFromSource(item), QItemSelectionModel::Select);
	});
}

void Widget::mutate()
{
	auto& p = static_cast<Application*>(qApp)->project();
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