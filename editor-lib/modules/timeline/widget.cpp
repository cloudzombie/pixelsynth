#include "widget.h"
#include "model.h"

#include <core/utils.h>

using Editor::Modules::Timeline::Widget;

using namespace Core;

Widget::Widget(QWidget* parent)
	: QWidget(parent)
	, tree_(new QTreeView(this))
	, model_(std::make_shared<Model>())
{
	p.setMutationCallback([&](auto mutationInfo)
	{
		auto selection = selectedNodes();
		model_->apply(mutationInfo);
		setSelection(selection);
	});

	tree_->setModel(model_.get());
	tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);

	mutate();
}

Widget::selection_t Widget::selectedNodes() const noexcept
{
	selection_t s;

	for (auto&& index: tree_->selectionModel()->selectedIndexes())
	{
		s.push_back(model_->uuidFromIndex(index));
	}

	return s;
}

void Widget::setSelection(selection_t selection) noexcept
{
	for (auto&& node: selection)
	{
		tree_->selectionModel()->select(model_->indexFromUuid(node), QItemSelectionModel::SelectCurrent);
	}
}

void Widget::mutate()
{
	switch (mutationIndex++)
	{
	case 0:
		p.mutate([&](auto& mut)
		{
			mut.append({ makeNode(hash("DummyNode"), "a") });
		});
		break;
	case 1:
		p.mutate([&](auto& mut) { mut.insertBefore(findNode(p, "a"), { makeNode(hash("DummyNode"), "b") }); });
		p.mutate([&](auto& mut) { mut.insertBefore(findNode(p, "b"), { makeNode(hash("DummyNode"), "c") }); });
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