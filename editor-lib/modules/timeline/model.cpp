#include "model.h"

#include <core/mutation_info.h>
#include <core/utils.h>

using Editor::Modules::Timeline::Model;
using ChangeType = Core::MutationInfo::ChangeType;
using Core::Node;
using Core::NodePtr;
using Core::Property;
using Core::PropertyPtr;
using Core::PropertyValue;
using Core::Uuid;

///

struct PropertyValueAsEditRole
{
	template <typename T>
	QVariant operator()(const T& t)
	{
		return QVariant(t);
	}
};

template <> QVariant PropertyValueAsEditRole::operator()<glm::vec2>(const glm::vec2& t) { return QVariant(QVector2D(t.x, t.y)); }
template <> QVariant PropertyValueAsEditRole::operator()<glm::vec3>(const glm::vec3& t) { return QVariant(QVector3D(t.x, t.y, t.z)); }
template <> QVariant PropertyValueAsEditRole::operator()<std::string>(const std::string& t) { return QVariant(t.c_str()); }

///

struct PropertyValueAsDisplayRole
{
	template <typename T>
	QVariant operator()(const T& t)
	{
		return QVariant(t);
	}
};

static QString doubleToString(double value, int decimals)
{
	QString str = QLocale::system().toString(value, 'f', decimals);
	if (qAbs(value) >= 1000.0) str.remove(QLocale::system().groupSeparator());
	return str;
}

template <> QVariant PropertyValueAsDisplayRole::operator()<glm::vec2>(const glm::vec2& t) { return QString("%1 %2").arg(doubleToString(t.x, 3)).arg(doubleToString(t.y, 3)); }
template <> QVariant PropertyValueAsDisplayRole::operator()<glm::vec3>(const glm::vec3& t) { return QString("%1 %2 %3").arg(doubleToString(t.x, 3)).arg(doubleToString(t.y, 3)).arg(doubleToString(t.z, 3)); }
template <> QVariant PropertyValueAsDisplayRole::operator()<std::string>(const std::string& t) { return t.c_str(); }

///

struct EditRoleAsPropertyValue
{
	QVariant v;

	explicit EditRoleAsPropertyValue(QVariant v)
		: v(v)
	{}

	template <typename T>
	PropertyValue operator()(const T& type)
	{
		return v.value<T>();
	}
};

template <> PropertyValue EditRoleAsPropertyValue::operator()<glm::vec2>(const glm::vec2& t) { return glm::vec2(v.value<QVector2D>().x(), v.value<QVector2D>().y()); }
template <> PropertyValue EditRoleAsPropertyValue::operator()<glm::vec3>(const glm::vec3& t) { return glm::vec3(v.value<QVector3D>().x(), v.value<QVector3D>().y(), v.value<QVector3D>().z()); }
template <> PropertyValue EditRoleAsPropertyValue::operator()<std::string>(const std::string& t) { return v.value<QString>().toStdString(); }

///

class PropertyValueItem: public QStandardItem
{
public:
	explicit PropertyValueItem(const Model* model, const Property* prop)
		: model_(model)
		, prop_(prop)
	{
		setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
		LOG->debug("Creating PropertyValueItem for {}", *prop);
	}

	void update(const Property* prop)
	{
		prop_ = prop;
		emitDataChanged();
	}

	QVariant data(int role) const override
	{
		switch (role)
		{
		case Qt::DisplayRole:
			return apply(PropertyValueAsDisplayRole(), prop_->getPropertyValue(0));
		case Qt::EditRole:
			return apply(PropertyValueAsEditRole(), prop_->getPropertyValue(0));
		case Qt::SizeHintRole:
			return QSize(0, 24);
		default:
			return QStandardItem::data(role);
		}
	}

	PropertyValue roundTrip() const
	{
		QVariant qv = apply(PropertyValueAsEditRole(), prop_->getPropertyValue(0));
		return apply(EditRoleAsPropertyValue(qv), prop_->getPropertyValue(0));
	}

private:
	void setData(const QVariant& value, int role) override
	{
		if (role != Qt::EditRole)
		{
			return QStandardItem::setData(value, role);
		}

		model_->emitPropertyChanged(prop_, apply(EditRoleAsPropertyValue(value), prop_->getPropertyValue(0)));
	}

	const Model* model_;
	const Property* prop_;
};

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
		, propertyValueItem_(new PropertyValueItem(model, prop.get()))
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

Model::Model()
{
}

QModelIndexList Model::apply(std::shared_ptr<Core::MutationInfo> mutation, const QModelIndexList& oldSelection) noexcept
{
	emit documentMutated(&mutation->prev, &mutation->cur);

	using NodeOrProperty = eggs::variant<NodePtr, PropertyPtr>;
	std::unordered_map<NodeOrProperty, NodeOrProperty> mutated;

	std::vector<ModelItem*> selection;
	for (auto&& index : oldSelection) selection.emplace_back(reinterpret_cast<ModelItem*>(itemFromIndex(index)));

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

	auto setRow = [&](QStandardItem* parent, size_t row, QList<QStandardItem*>&& items, RowType rowType)
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
				auto item = findItem(resolve(mut.prev));
				assert(item);
				auto prevIndex = findChildIndex(prevParentNode, item);
				assert(prevIndex != -1);

				if (prevIndex != mut.curIndex || prevParentNode != curParentNode)
				{
					setRow(curParentNode, mut.curIndex, prevParentNode->takeRow(prevIndex), rowType);
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

	// Generate new selection indices, perhaps based on mutated nodes
	QModelIndexList newSelection;
	for (auto nodeOrProperty : selection) newSelection.append(nodeOrProperty->index());
	return newSelection;
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
	{
		switch (section)
		{
		case Columns::Item:
			return "Item";
		case Columns::Value:
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


NodePtr Model::nodeFromIndex(const QModelIndex& index) const noexcept
{
	return ModelItem::node(itemFromIndex(index));
}

PropertyPtr Model::propertyFromIndex(const QModelIndex& index) const noexcept
{
	return ModelItem::prop(itemFromIndex(index));
}

PropertyValue Model::roundTripPropertyValueFromIndex(const QModelIndex& index) const noexcept
{
	auto item = static_cast<PropertyValueItem*>(itemFromIndex(index));
	return item->roundTrip();
}

void Model::emitPropertyChanged(const Property* prop, PropertyValue newValue) const noexcept
{
	emit propertyChanged(prop, newValue);
}