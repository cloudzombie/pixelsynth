#include "model.h"
#include "../property_editors/property_value_item.h"

#include <core/mutation_info.h>
#include <core/utils.h>

using Editor::Modules::Timeline::Model;
using Editor::Modules::PropertyEditors::PropertyValueItem;
using ChangeType = Core::MutationInfo::ChangeType;
using Core::Node;
using Core::NodePtr;
using Core::Property;
using Core::PropertyPtr;
using Core::PropertyValue;
using Core::Uuid;

///

class Model::ModelItem: public QStandardItem
{
public:
	explicit ModelItem(const Model* model, NodePtr node)
		: model_(model)
	{
		setFlags(flags_);
		update(node);
	}

	explicit ModelItem(const Model* model, PropertyPtr prop)
		: model_(model)
		, propertyValueItem_(new PropertyValueItem(prop.get()))
	{
		setFlags(flags_);
		update(prop);
	}

	QVariant data(int role) const override
	{
		switch (role)
		{
		case Qt::SizeHintRole:
			return QSize(0, 24);
		default:
			return QStandardItem::data(role);
		}
	}

	void update(NodePtr node)
	{
		auto prev = node_;
		node_ = node;
		setData(QVariant::fromValue(node_), static_cast<int>(ModelItemRoles::Data));
		setData(QVariant::fromValue<int>(static_cast<int>(ModelItemDataType::Node)), static_cast<int>(ModelItemRoles::Type));
		setData(Core::prop<std::string>(*node, "$Title", 0).c_str(), Qt::DisplayRole);
		emit model_->modelItemNodeMutated(prev, node);
	}

	void update(PropertyPtr prop)
	{
		auto prev = prop_;
		prop_ = prop;
		setData(QVariant::fromValue(prop_), static_cast<int>(ModelItemRoles::Data));
		setData(QVariant::fromValue<int>(static_cast<int>(ModelItemDataType::Property)), static_cast<int>(ModelItemRoles::Type));
		setData(prop->metadata().title().c_str(), Qt::DisplayRole);
		propertyValueItem_->update(prop.get());
		emit model_->modelItemPropertyMutated(prev, prop);
	}

	NodePtr node() const { return node_; }
	PropertyPtr prop() const { return prop_; }
	PropertyValueItem* propertyValueItem() const { return propertyValueItem_; }

	static NodePtr node(QStandardItem* item)
	{
		if (item->data(static_cast<int>(ModelItemRoles::Data)) == QVariant()) return nullptr;
		return reinterpret_cast<ModelItem*>(item)->node_;
	}

	static PropertyPtr prop(QStandardItem* item)
	{
		if (item->data(static_cast<int>(ModelItemRoles::Data)) == QVariant()) return nullptr;
		return reinterpret_cast<ModelItem*>(item)->prop_;
	}

private:
	const Model* model_;
	NodePtr node_ {};
	PropertyPtr prop_ {};
	PropertyValueItem* propertyValueItem_ {};

	const Qt::ItemFlags flags_ { Qt::ItemIsSelectable | Qt::ItemIsEnabled };
};

///

std::vector<QStandardItem*> Model::apply(std::shared_ptr<Core::MutationInfo> mutation) noexcept
{
	emit documentMutated(&mutation->prev, &mutation->cur);

	std::vector<QStandardItem*> removedItems;

	// Update the pointers stored in the model items
	using NodeOrProperty = eggs::variant<NodePtr, PropertyPtr>;
	std::unordered_map<NodeOrProperty, NodeOrProperty> mutated;
	auto updatePointers = [&](auto& changes)
	{
		for (auto&& mut: changes)
		{
			if (mut.type != ChangeType::Mutated) continue;

			findItem(mut.prev)->update(mut.cur);
			mutated.insert({ mut.prev, mut.cur });
		}
	};
	updatePointers(mutation->nodes);
	updatePointers(mutation->properties);

	auto resolve = [&](auto item) -> decltype(item)
	{
		if (!item) return item;

		auto it = mutated.find(item);
#ifdef _MSC_VER
		if (it != cend(mutated)) return *it->second.target<decltype(item)>();
#else
		if (it != cend(mutated)) return *it->second.template target<decltype(item)>();
#endif
		return item;
	};

	enum class RowType { Node, Property };

	auto setRow = [&](QStandardItem* parent, size_t row, QList<QStandardItem*> items, RowType rowType)
	{
		if (rowType == RowType::Property)
		{
			// Properties are indexed from 0, but they should always go below the node children of the parent
			// So increase the row offset by the number of children the parent has
			row += mutation->cur.childCount(*ModelItem::node(parent));
		}

		// When inserting a new item, make sure we above any items that have a higher index in the actual document
		// If we are inserting a node, also make sure we stay above any properties
		auto maxRow = row;
		for (row = 0; row < maxRow; row++)
		{
			if (row == static_cast<size_t>(parent->rowCount())) break;

			auto child = parent->child(row);

			// Encountered a property, so stop
			if (rowType == RowType::Node && ModelItem::prop(child)) break;

			size_t index;

			// Encountered a property that has a higher index than this item, so stop
			auto prop = ModelItem::prop(child);
			if (prop)
			{
				auto propNode = mutation->cur.parent(*prop);
				// add node childCount because the properties start BELOW the child nodes
				index = mutation->cur.childIndex(*prop) + mutation->cur.childCount(*propNode);
				if (index > maxRow) break;
			}

			// Encountered a node that has a higher index than this item, so stop
			auto node = ModelItem::node(child);
			if (node)
			{
				index = mutation->cur.childIndex(*node);
				if (index > maxRow) break;
			}
		}

		//LOG->debug("row: {}, rowCount: {}", row, parent->rowCount());
		if (row <= static_cast<size_t>(parent->rowCount())) parent->insertRow(row, items);
		else parent->appendRow(items);
	};

	auto applyMutations = [&](auto& changes, RowType rowType, auto createItems, bool onlyRemove)
	{
		for (auto&& mut : changes)
		{
			QStandardItem* prevParentNode = findItem(resolve(mut.prevParent));
			if (!prevParentNode && rowType == RowType::Node) prevParentNode = invisibleRootItem();
			QStandardItem* curParentNode = findItem(resolve(mut.curParent));
			if (!curParentNode && rowType == RowType::Node) curParentNode = invisibleRootItem();

			switch (mut.type)
			{
			case ChangeType::Added:
			{
				if (onlyRemove) continue;
				LOG->debug("Adding at position {}: {}", mut.curIndex, *mut.cur);
				setRow(curParentNode, mut.curIndex, createItems(mut.cur), rowType);
				break;
			}
			case ChangeType::Removed:
			{
				if (!onlyRemove) continue;
				auto item = findItem(mut.prev);
				if (!item) continue; // maybe was already deleted when parent was removed
				removedItems.push_back(item);
				auto childIndex = findChildIndex(prevParentNode, item);
				if (childIndex != -1) prevParentNode->removeRow(childIndex);
				LOG->debug("Removed at position {}: {}", childIndex, *mut.prev);
				break;
			}
			case ChangeType::Mutated:
			{
				if (onlyRemove) continue;
				if (!prevParentNode) prevParentNode = curParentNode;
				else if (!curParentNode) curParentNode = prevParentNode;
				assert(prevParentNode && curParentNode);

				LOG->debug("Mutating from position {} to position {}: {}", mut.prevIndex, mut.curIndex, *mut.cur);
				auto nodeOrProperty = resolve(mut.prev);
				auto item = findItem(nodeOrProperty);
				assert(item);
				auto prevIndex = findChildIndex(prevParentNode, item);
				assert(prevIndex != -1);

				if (prevIndex != mut.curIndex || prevParentNode != curParentNode)
				{
					// Move the row
					auto itemRowItems = prevParentNode->takeRow(prevIndex);
					setRow(curParentNode, mut.curIndex, itemRowItems, rowType);
				}

				break;
			}
			}
		}
	};

	auto createNodeItems = [&](NodePtr node)
	{
		QList<QStandardItem*> items;
		items << new ModelItem(this, node) << nullptr;
		return items;
	};
	auto createPropertyItems = [&](PropertyPtr prop)
	{
		QList<QStandardItem*> items;
		auto modelItem = new ModelItem(this, prop);
		items << modelItem << modelItem->propertyValueItem();
		return items;
	};
	applyMutations(mutation->nodes, RowType::Node, createNodeItems, false);
	applyMutations(mutation->properties, RowType::Property, createPropertyItems, false);
	applyMutations(mutation->nodes, RowType::Node, createNodeItems, true);
	applyMutations(mutation->properties, RowType::Property, createPropertyItems, true);

	return removedItems;
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
	{
		switch (section)
		{
		case static_cast<int>(Columns::Item):
			return "Item";
		case static_cast<int>(Columns::Value):
			return "Value";
		default:
			return "Unknown";
		}
	}
	default:
		return QVariant();
	}
}

int Model::findChildIndex(QStandardItem* parent, ModelItem* item) noexcept
{
	for (int row = 0; row < parent->rowCount(); row++)
	{
		if (parent->child(row) == item) return row;
	}
	return -1;
}

QModelIndex Model::findItemIndex(Core::NodePtr ptr) const noexcept
{
	auto item = findItem(ptr);
	if (!item) return QModelIndex();
	return item->index();
}

Model::ModelItem* Model::findItem(QVariant ptr) const noexcept
{
	auto results = match(
		index(0, 0),
		static_cast<int>(ModelItemRoles::Data),
		ptr,
		1,
		Qt::MatchRecursive);

	if (results.isEmpty()) return nullptr;
	return static_cast<ModelItem*>(itemFromIndex(results.at(0)));
}

Model::ModelItem* Model::findItem(NodePtr ptr) const noexcept { return findItem(QVariant::fromValue(ptr)); }
Model::ModelItem* Model::findItem(PropertyPtr ptr) const noexcept { return findItem(QVariant::fromValue(ptr)); }

QSet<QStandardItem*> Model::indicesToItems(const QModelIndexList& indices) const noexcept
{
	QSet<QStandardItem*> items;
	for (auto&& index : indices)
	{
		if (index.column() != 0) continue;
		items.insert(itemFromIndex(index));
	}
	return items;
}

QModelIndexList Model::itemsToIndices(const QSet<QStandardItem*> items) const noexcept
{
	QModelIndexList list;
	for (auto&& item : items)
	{
		list.push_back(indexFromItem(item));
	}
	return list;
}

NodePtr Model::nodeFromIndex(const QModelIndex& index) const noexcept
{
	if (!index.isValid()) return nullptr;
	auto item = itemFromIndex(index);
	if (!item) return nullptr;
	return ModelItem::node(item);
}

PropertyPtr Model::propertyFromIndex(const QModelIndex& index) const noexcept
{
	if (!index.isValid()) return nullptr;
	auto item = itemFromIndex(index);
	if (!item) return nullptr;
	return ModelItem::prop(item);
}

PropertyValue Model::roundTripPropertyValueFromIndex(const QModelIndex& index) const noexcept
{
	auto item = static_cast<PropertyValueItem*>(itemFromIndex(index));
	return item->roundTrip();
}