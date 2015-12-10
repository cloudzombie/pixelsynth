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

enum class WhichHandle { Start, Stop };

using pressed_fn_t = std::function<void(bool)>;
using dragged_fn_t = std::function<void(WhichHandle, const int)>;
using released_fn_t = std::function<void()>;

class DragHandle: public QWidget
{
public:
	DragHandle(QWidget* parent, WhichHandle which, pressed_fn_t pressedFn, dragged_fn_t draggedFn, released_fn_t releasedFn)
		: QWidget(parent)
		, which_(which)
		, pressedFn_(pressedFn)
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
			pressedFn_(event->modifiers() & Qt::ControlModifier);
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

	WhichHandle which_;
	pressed_fn_t pressedFn_;
	dragged_fn_t draggedFn_;
	released_fn_t releasedFn_;

	int dragX_;
	bool isDragging_ {};
};

class SelectionArea: public QWidget
{
public:
	SelectionArea(QWidget* parent, pressed_fn_t pressedFn, dragged_fn_t draggedFn, released_fn_t releasedFn)
		: QWidget(parent)
		, pressedFn_(pressedFn)
		, draggedFn_(draggedFn)
		, releasedFn_(releasedFn)
	{
	}

private:
	void mousePressEvent(QMouseEvent* event) override
	{
		if (event->button() == Qt::LeftButton)
		{
			isDragging_ = true;
			dragX_ = event->globalPos().x();
			pressedFn_(event->modifiers() & Qt::ControlModifier);
		}
	}

	void mouseMoveEvent(QMouseEvent* event) override
	{
		if (event->buttons() & Qt::LeftButton && isDragging_)
		{
			int pos = event->globalPos().x() - dragX_;
			dragX_ = event->globalPos().x();
			draggedFn_(WhichHandle::Start, pos);
			draggedFn_(WhichHandle::Stop, pos);
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

	pressed_fn_t pressedFn_;
	dragged_fn_t draggedFn_;
	released_fn_t releasedFn_;

	int dragX_;
	bool isDragging_ {};
};

KeyframeNodeWidget::KeyframeNodeWidget(const KeyframeDelegate& kd, Project& project, const Model& model, const KeyframeSelectionModel& selectionModel, NodePtr node, QWidget* parent)
	: QWidget(parent)
	, model_(model)
	, node_(node)
{
	setFixedWidth(1000);

	auto updateGeometry = [this]()
	{
		area_->setFixedWidth(stop_ - start_);
		area_->move(start_, area_->pos().y());
		startHandle_->move(0 - startHandle_->width() * 0.5, 0);
		stopHandle_->move(area_->width() - stopHandle_->width() * 0.5, 0);
	};

	auto updateSelectionArea = [this](bool selected)
	{
		QString color = selected ? "888" : "666";
		area_->setStyleSheet(QString("background-color: #" + color));
	};

	///

	auto pressed = [&](bool multiSelect) { emit kd.nodeClicked(node_, multiSelect); };

	auto dragged = [this, &kd, updateGeometry](WhichHandle which, const int offset)
	{
		Core::Frame offsetFrames = offset;

		switch (which)
		{
		case WhichHandle::Start:
			emit kd.nodeDragged({ offsetFrames, 0.0f });
			break;
		case WhichHandle::Stop:
			emit kd.nodeDragged({ 0.0f, offsetFrames });
			break;
		}
	};

	auto released = [this, &kd]()
	{
		auto vis = node_->visibility();
		if (fabs(start_ - vis.first) > 0.1 || fabs(stop_ - vis.second) > 0.1)
		{
			emit kd.nodeReleased({ start_ - vis.first, stop_ - vis.second });
		}
	};

	///

	area_ = new SelectionArea(this, pressed, dragged, released);
	updateSelectionArea(selectionModel.isSelected(node));
	area_->show();

	startHandle_ = new DragHandle(area_, WhichHandle::Start, pressed, dragged, released);
	startHandle_->setStyleSheet("background-color: #f0f;");
	stopHandle_ = new DragHandle(area_, WhichHandle::Stop, pressed, dragged, released);
	stopHandle_->setStyleSheet("background-color: #f0f;");
	startHandle_->show();
	stopHandle_->show();

	connect(&selectionModel, &KeyframeSelectionModel::selectionChanged, this, [this, updateSelectionArea](NodePtr n, bool selected)
	{
		if (n != node_) return;
		updateSelectionArea(selected);
	});

	connect(&selectionModel, &KeyframeSelectionModel::selectionMoved, this, [this, updateGeometry](NodePtr n, const std::pair<Core::Frame, Core::Frame> offsets)
	{
		if (n != node_) return;
		start_ += offsets.first;
		stop_ += offsets.second;
		updateGeometry();
	});

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