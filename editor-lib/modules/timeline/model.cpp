#include "model.h"

#include <core/mutation_info.h>
#include <core/utils.h>

using Editor::Modules::Timeline::Model;
using ChangeType = Core::MutationInfo::ChangeType;
using NodePtr = Core::NodePtr;

Model::Model()
{
}

void Model::apply(std::shared_ptr<Core::MutationInfo> mutation)
{
	doc_ = &mutation->cur;

	for (auto&& node: mutation->nodes)
	{
		switch (node.type)
		{
		case ChangeType::Added:
		{
			beginInsertRows(createIndex(0, 0, toId(mutation->cur.parent(node.cur))), node.index, node.index);
			endInsertRows();
			break;
		}
		case ChangeType::Removed:
		{
			beginRemoveRows(createIndex(0, 0, toId(mutation->prev.parent(node.prev))), node.index, node.index);
			endRemoveRows();
			break;
		}
		case ChangeType::Mutated: break;
		default: break;
		}
	}
}

NodePtr Model::fromId(quintptr id) const noexcept
{
	return ptrIds_[id];
}

quintptr Model::toId(const NodePtr& ptr) const noexcept
{
	for (auto&& kvp: ptrIds_)
	{
		if (kvp.second == ptr) return kvp.first;
	}
	ptrIds_[++id_] = ptr;
	return id_;
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
	if (row < 0 || column < 0) return QModelIndex();
	if (row >= rowCount(parent)) return QModelIndex();

	auto parentNode = parent.isValid() ? fromId(parent.internalId()) : *doc_->nodes().begin();
	return createIndex(row, column, toId(doc_->child(parentNode, row)));
}

QModelIndex Model::parent(const QModelIndex& child) const
{
	if (!child.isValid()) return QModelIndex();
	auto childNode = fromId(child.internalId());
	auto parentNode = doc_->parent(childNode);
	if (parentNode == *doc_->nodes().begin()) return QModelIndex();
	return createIndex(doc_->childIndex(parentNode), 0, toId(parentNode));
}

int Model::rowCount(const QModelIndex& parent) const
{
	if (parent.column() >= 1) return 0; // invalid column

	auto parentNode = parent.isValid() ? fromId(parent.internalId()) : *doc_->nodes().begin();
	return doc_->childCount(parentNode);
}

int Model::columnCount(const QModelIndex& parent) const
{
	return 1;
}

QVariant Model::data(const QModelIndex& index, int role) const
{
	if (!index.isValid()) return QVariant();

	auto node = fromId(index.internalId());
	switch (role)
	{
	case Qt::DisplayRole:
		return prop(node, "$Title")->get<std::string>(0).c_str();
	default:
		return QVariant();
	}
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (section)
	{
	default:
		return "Header";
	}
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const
{
	if (index == QModelIndex()) return 0;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
