#include "model.h"

#include <core/mutation_info.h>
#include <core/utils.h>

using Editor::Modules::Timeline::Model;
using ChangeType = Core::MutationInfo::ChangeType;
using NodePtr = Core::NodePtr;
using Uuid = Core::Uuid;

const size_t NodeRole = Qt::UserRole;
const size_t UuidRole = Qt::UserRole + 1;

Model::Model()
{
}

void Model::apply(std::shared_ptr<Core::MutationInfo> mutation) noexcept
{
	for (auto&& mut: mutation->nodes)
	{
		switch (mut.type)
		{
		case ChangeType::Added:
		{
			QStandardItem* parentNode = findItem(mutation->cur.parent(mut.cur));
			if (!parentNode) parentNode = invisibleRootItem();

			parentNode->insertRow(mut.index, makeItem(mut.cur));

			break;
		}
		case ChangeType::Removed:
		{
			auto prevItem = findItem(mut.prev);
			QStandardItem* prevParentNode = findItem(mutation->prev.parent(mut.prev));
			if (!prevParentNode) prevParentNode = invisibleRootItem();

			prevParentNode->removeRow(findChildIndex(prevParentNode, prevItem));
			break;
		}
		case ChangeType::Mutated:
		{
			auto prevItem = findItem(mut.prev);
			QStandardItem* prevParentNode = findItem(mutation->prev.parent(mut.prev));
			if (!prevParentNode) prevParentNode = invisibleRootItem();

			QStandardItem* curParentNode = findItem(mutation->cur.parent(mut.cur));
			if (!curParentNode) curParentNode = invisibleRootItem();

			prevParentNode->removeRow(findChildIndex(prevParentNode, prevItem));
			curParentNode->insertRow(mut.index, makeItem(mut.cur));

			break;
		}
		default: break;
		}
	}
}

Uuid Model::uuidFromIndex(const QModelIndex& index) const noexcept
{
	auto node = reinterpret_cast<Core::Node*>(itemFromIndex(index)->data(NodeRole).value<void*>());
	return node->uuid();
}

QModelIndex Model::indexFromUuid(const Uuid& nodeUuid) const noexcept
{
	return indexFromItem(findItem(nodeUuid));
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
	item->setData(QVariant::fromValue<Uuid>(node->uuid()), UuidRole);
	item->setData(Core::prop<std::string>(*node, "$Title", 0).c_str(), Qt::DisplayRole);
	return item;
}

int Model::findChildIndex(QStandardItem* parent, QStandardItem* item) const noexcept
{
	for (int row = 0; row < parent->rowCount();row++)
	{
		if (parent->child(row) == item) return row;
	}
	return -1;
}

QStandardItem* Model::findItem(const NodePtr& node) const noexcept
{
	auto result = match(
		index(0, 0),
		NodeRole,
		QVariant::fromValue<void*>(const_cast<void*>(static_cast<const void*>(node.get()))),
		1,
		Qt::MatchRecursive);
	if (result.isEmpty()) return nullptr;
	return itemFromIndex(result.at(0));
}

QStandardItem* Model::findItem(const Uuid& nodeUuid) const noexcept
{
	auto result = match(
		index(0, 0),
		UuidRole,
		QVariant::fromValue<Uuid>(nodeUuid),
		1,
		Qt::MatchRecursive);
	if (result.isEmpty()) return nullptr;
	return itemFromIndex(result.at(0));
}