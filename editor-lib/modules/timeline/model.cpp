#include "model.h"

#include <core/mutation_info.h>
#include <core/utils.h>

using Editor::Modules::Timeline::Model;
using ChangeType = Core::MutationInfo::ChangeType;
using Node = Core::Node;
using Property = Core::Property;
using Uuid = Core::Uuid;

const size_t ModelItemDataRole = Qt::UserRole;

class Model::ModelItem: public QStandardItem
{
public:
	ModelItem(const Node* node)
		: node_(node)
	{
		update(node);
	}

	ModelItem(const Property* prop)
		: prop_(prop)
	{
		update(prop);
	}

	void update(const Node* node)
	{
		node_ = node;
		setData(QVariant::fromValue<void*>(const_cast<void*>(static_cast<const void*>(node_))), ModelItemDataRole);
		setData(Core::prop<std::string>(*node, "$Title", 0).c_str(), Qt::DisplayRole);
	}

	void update(const Property* prop)
	{
		prop_ = prop;
		setData(QVariant::fromValue<void*>(const_cast<void*>(static_cast<const void*>(prop_))), ModelItemDataRole);
		setData(prop->metadata().title().c_str(), Qt::DisplayRole);
	}

	const Node* node() const { return node_; }
	const Property* prop() const { return prop_; }

	static const Node* node(QStandardItem* item)
	{
		if (item->data(ModelItemDataRole) == QVariant()) return nullptr;
		return reinterpret_cast<ModelItem*>(item)->node_;
	}

	static const Property* prop(QStandardItem* item)
	{
		if (item->data(ModelItemDataRole) == QVariant()) return nullptr;
		return reinterpret_cast<ModelItem*>(item)->prop_;
	}

private:
	const Node* node_ {};
	const Property* prop_ {};
};

Model::Model()
{
}

QModelIndexList Model::apply(std::shared_ptr<Core::MutationInfo> mutation, const QModelIndexList& oldSelection) noexcept
{
	using NodeOrProperty = eggs::variant<const Node*, const Property*>;
	std::unordered_map<NodeOrProperty, NodeOrProperty> mutated;

	std::vector<ModelItem*> selection;
	for (auto&& index : oldSelection) selection.emplace_back(reinterpret_cast<ModelItem*>(itemFromIndex(index)));

	auto updatePointers = [&](auto& changes)
	{
		for (auto&& mut: changes)
		{
			if (mut.type != ChangeType::Mutated) continue;

			auto item = findItem(mut.prev.get());
			item->update(mut.cur.get());
			mutated.insert({ mut.prev.get(), mut.cur.get() });
		}
	};

	updatePointers(mutation->nodes);
	updatePointers(mutation->properties);

	auto resolve = [&](auto item) -> decltype(item)
	{
		auto it = mutated.find(item);
		if (it != cend(mutated)) return *it->second.target<decltype(item)>();
		return item;
	};

	auto setRow = [&](QStandardItem* parent, size_t row, QList<QStandardItem*>& items, bool isNode)
	{
		// When inserting a new item, make sure we above any items that have a higher index in the actual document
		// If we are inserting a node, also make sure we stay above any properties
		auto maxRow = row;
		for (row = 0; row < maxRow; row++)
		{
			if (row == static_cast<size_t>(parent->rowCount())) break;

			auto child = parent->child(row);

			// Encountered a property, so stop
			if (isNode && ModelItem::prop(child)) break;

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

	auto applyMutations = [&](auto& changes, bool isNode)
	{
		for (auto&& mut : changes)
		{
			QStandardItem* prevParentNode = findItem(resolve(mut.prevParent.get()));
			if (!prevParentNode && isNode) prevParentNode = invisibleRootItem();
			QStandardItem* curParentNode = findItem(resolve(mut.curParent.get()));
			if (!curParentNode && isNode) curParentNode = invisibleRootItem();

			switch (mut.type)
			{
			case ChangeType::Added:
			{
				LOG->debug("Adding at position {}: {}", mut.curIndex, *mut.cur);
				QList<QStandardItem*> items;
				auto i = new ModelItem(mut.cur.get());
				items.append(i);
				setRow(curParentNode, mut.curIndex, items, isNode);
				break;
			}
			case ChangeType::Removed:
			{
				auto item = findItem(mut.prev.get());
				if (!item) continue; // maybe was already deleted when parent was removed
				auto childIndex = findChildIndex(prevParentNode, item);
				if (childIndex != -1) prevParentNode->removeRow(childIndex);
				LOG->debug("Removed at position {}: {}", childIndex, *mut.prev);
				break;
			}
			case ChangeType::Mutated:
			{
				if (!prevParentNode) prevParentNode = curParentNode;
				else if (!curParentNode) curParentNode = prevParentNode;
				assert(prevParentNode && curParentNode);

				LOG->debug("Mutating to position {}: {}", mut.curIndex, *mut.cur);
				auto item = findItem(resolve(mut.prev.get()));
				assert(item);
				auto prevIndex = findChildIndex(prevParentNode, item);
				assert(prevIndex != -1);

				if (prevIndex != mut.curIndex || prevParentNode != curParentNode)
				{
					setRow(curParentNode, mut.curIndex, prevParentNode->takeRow(prevIndex), isNode);
				}
				break;
			}
			}
		}
	};

	applyMutations(mutation->nodes, true);
	applyMutations(mutation->properties, false);

	// Generate new selection indices, perhaps based on mutated nodes
	QModelIndexList newSelection;
	for (auto nodeOrProperty : selection) newSelection.append(nodeOrProperty->index());
	return newSelection;
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (section)
	{
	default:
		return "Header";
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

Model::ModelItem* Model::findItem(const void* ptr) const noexcept
{
	auto result = match(
		index(0, 0),
		ModelItemDataRole,
		QVariant::fromValue(const_cast<void*>(ptr)),
		1,
		Qt::MatchRecursive);
	if (result.isEmpty()) return nullptr;
	return static_cast<ModelItem*>(itemFromIndex(result.at(0)));
}

const Core::Node* Model::nodeFromIndex(const QModelIndex& index) const noexcept
{
	return ModelItem::node(itemFromIndex(index));
}

const Core::Property* Model::propertyFromIndex(const QModelIndex& index) const noexcept
{
	return ModelItem::prop(itemFromIndex(index));
}
