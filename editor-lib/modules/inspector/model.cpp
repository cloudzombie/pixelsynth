#include "model.h"
#include "../property_editors/property_value_item.h"
#include <core/node.h>
#include <core/metadata.h>

using Editor::Modules::Inspector::Model;
using Editor::Modules::PropertyEditors::PropertyValueItem;

using propertygroup_t = std::unordered_map<Core::PropertyMetadata, std::unordered_set<Core::PropertyPtr>>;

struct Model::Impl
{
	propertygroup_t group;
};

class Model::ModelItem: public QStandardItem
{
public:
	ModelItem(const Core::PropertyMetadata metadata)
		: metadata_(metadata)
	{}

	QVariant data(int role) const override
	{
		switch (role)
		{
		case Qt::DisplayRole:
			return metadata_.title().c_str();
		case Qt::SizeHintRole:
			return QSize(0, 24);
		default:
			return QStandardItem::data(role);
		}
	}

	const Core::PropertyMetadata metadata_;
};

Model::Model()
	: impl_(std::make_unique<Impl>())
{}

void Model::selectNode(Core::NodePtr node)
{
	nodes_.insert(node);
	update();
}

void Model::deselectNode(Core::NodePtr node)
{
	nodes_.erase(node);
	update();
}

void Model::update()
{
	propertygroup_t newGroup;
	for (auto&& node: nodes_)
	{
		for (auto&& prop: node->properties())
		{
			newGroup[prop->metadata()].insert(prop);
		}
	}

	// We should only show properties that are shared by all selected nodes
	if (nodes_.size() > 1)
	{
		auto copy = newGroup;
		for (auto&& propGroup : copy)
		{
			if (propGroup.second.size() != nodes_.size()) newGroup.erase(propGroup.first);
		}
	}

	// Add any new properties
	for (auto&& propGroup : newGroup)
	{
		if (impl_->group.find(propGroup.first) == end(impl_->group))
		{
			QList<QStandardItem*> items;
			auto item = new ModelItem(propGroup.first);
			items << item << new PropertyValueItem(propGroup.second.begin()->get());
			appendRow(items);
		}
	}

	// Remove old ones
	int row = 0;
	for (auto&& propGroup: impl_->group)
	{
		if (newGroup.find(propGroup.first) == end(newGroup))
		{
			invisibleRootItem()->removeRow(row);
		}
		else
		{
			row++;
		}
	}

	impl_->group = newGroup;
}