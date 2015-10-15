#include "model.h"

#include <core/mutation_info.h>
#include <core/utils.h>

using Editor::Modules::Timeline::Model;
using ChangeType = Core::MutationInfo::ChangeType;
using Node = Core::Node;
using Uuid = Core::Uuid;

const size_t NodeRole = Qt::UserRole;

Model::Model()
{
}

QModelIndexList Model::apply(std::shared_ptr<Core::MutationInfo> mutation, const QModelIndexList& oldSelection) noexcept
{
	std::unordered_map<const Node*, const Node*> mutatedNodes;

	std::vector<const Node*> selection;
	for (auto&& index : oldSelection) selection.emplace_back(nodeFromIndex(index));

	for (auto&& mut : mutation->nodes)
	{
		switch (mut.type)
		{
		case ChangeType::Added:
		{
			QStandardItem* parentNode = findItem(*mutation->cur.parent(mut.cur));
			if (!parentNode) parentNode = invisibleRootItem();

			parentNode->insertRow(mut.index, makeItem(mut.cur));
			break;
		}
		case ChangeType::Removed:
		{
			auto prevItem = findItem(*mut.prev);
			QStandardItem* prevParentNode = findItem(*mutation->prev.parent(mut.prev));
			if (!prevParentNode) prevParentNode = invisibleRootItem();

			prevParentNode->removeRow(findChildIndex(prevParentNode, prevItem));

			break;
		}
		case ChangeType::Mutated:
		{
			auto prevItem = findItem(*mut.prev);
			QStandardItem* prevParentNode = findItem(*mutation->prev.parent(mut.prev));
			if (!prevParentNode) prevParentNode = invisibleRootItem();

			QStandardItem* curParentNode = findItem(*mutation->cur.parent(mut.cur));
			if (!curParentNode) curParentNode = invisibleRootItem();

			auto curItem = makeItem(mut.cur);
			prevParentNode->removeRow(findChildIndex(prevParentNode, prevItem));
			curParentNode->insertRow(mut.index, curItem);

			mutatedNodes.insert({ mut.prev.get(), mut.cur.get() });

			break;
		}
		default: break;
		}
	}

	// Generate new selection indices, perhaps based on mutated nodes
	QModelIndexList newSelection;
	for (auto node: selection)
	{
		auto it = mutatedNodes.find(node);
		if (it != cend(mutatedNodes)) node = it->second;

		auto index = indexFromNode(*node);
		newSelection.append(index);
	}
	return newSelection;
}

Core::Node* Model::nodeFromIndex(const QModelIndex& index) const noexcept
{
	return reinterpret_cast<Core::Node*>(itemFromIndex(index)->data(NodeRole).value<void*>());
}

QModelIndex Model::indexFromNode(const Core::Node& node) const noexcept
{
	return indexFromItem(findItem(node));
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (section)
	{
	default:
		return "Header";
	}
}

QStandardItem* Model::makeItem(const Core::NodePtr& node) const noexcept
{
	auto item = new QStandardItem();
	item->setData(QVariant::fromValue<void*>(const_cast<void*>(static_cast<const void*>(node.get()))), NodeRole);
	item->setData(Core::prop<std::string>(*node, "$Title", 0).c_str(), Qt::DisplayRole);
	return item;
}

int Model::findChildIndex(QStandardItem* parent, QStandardItem* item) const noexcept
{
	for (int row = 0; row < parent->rowCount(); row++)
	{
		if (parent->child(row) == item) return row;
	}
	return -1;
}

QStandardItem* Model::findItem(const Core::Node& node) const noexcept
{
	auto result = match(
		index(0, 0),
		NodeRole,
		QVariant::fromValue<void*>(const_cast<void*>(static_cast<const void*>(&node))),
		1,
		Qt::MatchRecursive);
	if (result.isEmpty()) return nullptr;
	return itemFromIndex(result.at(0));
}