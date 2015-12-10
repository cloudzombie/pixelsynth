#include <core/node.h>
#include <core/project.h>
#include "keyframe_delegate.h"
#include "keyframe_selectionmodel.h"
#include "model.h"

using Core::Document;
using Core::Frame;
using Core::Node;
using Core::NodePtr;
using Core::Project;
using Core::Property;
using Core::PropertyPtr;
using Editor::Modules::Timeline::KeyframeNodeWidget;
using Editor::Modules::Timeline::KeyframePropertyWidget;
using Editor::Modules::Timeline::KeyframeDelegate;
using Editor::Modules::Timeline::KeyframeSelectionModel;
using Editor::Modules::Timeline::Model;
using ModelItemRoles = Model::ModelItemRoles;
using ModelItemDataType = Model::ModelItemDataType;

///

class DragHandle: public QWidget
{
public:
	enum class Which { Start, Stop };
	using dragged_fn_t = std::function<void(Which, const int)>;
	using released_fn_t = std::function<void()>;

	DragHandle(QWidget* parent, Which which, dragged_fn_t draggedFn, released_fn_t releasedFn)
		: QWidget(parent)
		, which_(which)
		, draggedFn_(draggedFn)
		, releasedFn_(releasedFn)
	{
		resize(5, parent->height());
		setCursor(Qt::SizeHorCursor);
		setMouseTracking(true);
	}

private:
	void mousePressEvent(QMouseEvent* event) override
	{
		if (event->button() == Qt::LeftButton)
		{
			isDragging_ = true;
			dragX_ = event->globalPos().x();
		}
	}

	void mouseMoveEvent(QMouseEvent* event) override
	{
		if (event->buttons() & Qt::LeftButton && isDragging_)
		{
			int pos = event->globalPos().x() - dragX_;
			dragX_ = event->globalPos().x();
			draggedFn_(which_, pos);
		}
	}

	void mouseReleaseEvent(QMouseEvent* event) override
	{
		if (event->button() == Qt::LeftButton)
		{
			isDragging_ = false;
			releasedFn_();
		}
	}

	Which which_;
	dragged_fn_t draggedFn_;
	released_fn_t releasedFn_;

	int dragX_;
	bool isDragging_ {};
};

class SelectionArea: public QWidget
{
public:
	using pressed_fn_t = std::function<void(QMouseEvent*, bool)>;

	SelectionArea(QWidget* parent, pressed_fn_t pressedFn)
		: QWidget(parent)
		, pressedFn_(pressedFn)
	{
	}

private:
	void mousePressEvent(QMouseEvent* event) override
	{
		auto multiSelect = event->modifiers() & Qt::ControlModifier;
		pressedFn_(event, multiSelect);
	}

	pressed_fn_t pressedFn_;
};

KeyframeNodeWidget::KeyframeNodeWidget(const KeyframeDelegate& kd, Project& project, const Model& model, const KeyframeSelectionModel& selectionModel, NodePtr node, QWidget* parent)
	: QWidget(parent)
	, model_(model)
	, node_(node)
{
	setFixedWidth(1000);

	area_ = new SelectionArea(this, [&](QMouseEvent* event, bool multiSelect) { emit kd.nodeClicked(node_, multiSelect); });

	auto updateSelectionArea = [this](bool selected)
	{
		QString color = selected ? "888" : "666";
		area_->setStyleSheet(QString("background-color: #" + color));
	};

	updateSelectionArea(selectionModel.isSelected(node));
	area_->show();

	connect(&selectionModel, &KeyframeSelectionModel::selectionChanged, this, [this, updateSelectionArea](NodePtr n, bool selected)
	{
		if (n != node_) return;
		updateSelectionArea(selected);
	});

	auto updateGeometry = [this]()
	{
		area_->setFixedWidth(stop_ - start_);
		area_->move(start_, area_->pos().y());
		startHandle_->move(0 - startHandle_->width() * 0.5, 0);
		stopHandle_->move(area_->width() - stopHandle_->width() * 0.5, 0);
	};

	auto dragged = [this, updateGeometry](DragHandle::Which which, const int offset)
	{
		switch (which)
		{
		case DragHandle::Which::Start:
			start_ += offset;
			break;
		case DragHandle::Which::Stop:
			stop_ += offset;
			break;
		}
		updateGeometry();
	};

	auto released = [this, &kd]()
	{
		auto vis = node_->visibility();
		emit kd.visibilityOffset({ start_ - vis.first, stop_ - vis.second });
	};

	startHandle_ = new DragHandle(area_, DragHandle::Which::Start, dragged, released);
	startHandle_->setStyleSheet("background-color: #f0f;");
	stopHandle_ = new DragHandle(area_, DragHandle::Which::Stop, dragged, released);
	stopHandle_->setStyleSheet("background-color: #f0f;");
	startHandle_->show();
	stopHandle_->show();

	auto updateNode = [&, updateGeometry](NodePtr prevNode, NodePtr curNode)
	{
		if (prevNode != node_) return;
		node_ = curNode;

		std::tie(start_, stop_) = node_->visibility();
		updateGeometry();
	};
	connect(&model, &Model::modelItemNodeMutated, this, updateNode);
	updateNode(node_, node_);

	auto updateDocument = [this, updateNode](const Document* prev, const Document* cur)
	{
		auto vis = cur->settings().visibility;
		setFixedWidth(vis.second - vis.first);
		updateNode(node_, node_);
	};
	connect(&model, &Model::documentMutated, this, updateDocument);
	updateDocument(&project.current(), &project.current());
}

///

KeyframePropertyWidget::KeyframePropertyWidget(const Model& model, PropertyPtr prop, QWidget* parent)
	: QWidget(parent)
	, model_(model)
	, prop_(prop)
{
	auto update = [=](PropertyPtr prevProperty, PropertyPtr curProperty)
	{
		if (prevProperty != prop_) return;
		prop_ = curProperty;
	};
	connect(&model, &Model::modelItemPropertyMutated, this, update);

	setStyleSheet("background-color: #00f;");
}

///

KeyframeDelegate::KeyframeDelegate(Project& project, const QSortFilterProxyModel& proxy, const Model& model, const KeyframeSelectionModel& selectionModel)
	: project_(project)
	, proxy_(proxy)
	, model_(model)
	, selectionModel_(selectionModel)
{
}

QWidget* KeyframeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	auto type = static_cast<ModelItemDataType>(proxy_.data(index, static_cast<int>(ModelItemRoles::Type)).value<int>());

	switch (type)
	{
	case ModelItemDataType::Node:
		return new KeyframeNodeWidget(*this, project_, model_, selectionModel_, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<NodePtr>(), parent);
	case ModelItemDataType::Property:
		return new KeyframePropertyWidget(model_, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<PropertyPtr>(), parent);
	}

	return nullptr;
}