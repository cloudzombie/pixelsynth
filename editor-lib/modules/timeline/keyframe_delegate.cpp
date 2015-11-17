#include "keyframe_delegate.h"
#include "model.h"

using Core::Node;
using Core::Property;
using Editor::Modules::Timeline::KeyframeNodeWidget;
using Editor::Modules::Timeline::KeyframePropertyWidget;
using Editor::Modules::Timeline::KeyframeDelegate;
using Editor::Modules::Timeline::Model;
using ModelItemRoles = Model::ModelItemRoles;
using ModelItemDataType = Model::ModelItemDataType;

///

KeyframeNodeWidget::KeyframeNodeWidget(const Model& model, const Node* node, QWidget* parent)
	: QWidget(parent)
	, model_(model)
	, node_(node)
{
	auto update = [&](const Node* prevNode, const Node* curNode)
	{
		if (prevNode != node_) return;
		node_ = curNode;
	};
	connect(&model, &Model::modelItemNodeMutated, this, update);

	setStyleSheet("background-color: #0f0;");
}

void KeyframeNodeWidget::mousePressEvent(QMouseEvent* event)
{
	setStyleSheet("background-color: #f00;");
	event->accept();
}

///

KeyframePropertyWidget::KeyframePropertyWidget(const Model& model, const Property* prop, QWidget* parent)
	: QWidget(parent)
	, model_(model)
	, prop_(prop)
{
	auto update = [=](const Property* prevProperty, const Property* curProperty)
	{
		if (prevProperty != prop_) return;
		prop_ = curProperty;
	};
	connect(&model, &Model::modelItemPropertyMutated, this, update);

	setStyleSheet("background-color: #00f;");
}

///

KeyframeDelegate::KeyframeDelegate(const QSortFilterProxyModel& proxy, const Model& model)
	: proxy_(proxy)
	, model_(model)
{
}

QWidget* KeyframeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	auto data = proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<void*>();
	auto type = static_cast<ModelItemDataType>(proxy_.data(index, static_cast<int>(ModelItemRoles::Type)).value<int>());

	switch (type)
	{
	case ModelItemDataType::Node:
		return new KeyframeNodeWidget(model_, static_cast<const Node*>(data), parent);
	case ModelItemDataType::Property:
		return new KeyframePropertyWidget(model_, static_cast<const Property*>(data), parent);
	}

	return nullptr;
}