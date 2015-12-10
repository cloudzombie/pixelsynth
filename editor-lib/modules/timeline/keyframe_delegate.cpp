#include <core/node.h>
#include <core/project.h>
#include "keyframe_delegate.h"
#include "model.h"

using Core::Document;
using Core::Frame;
using Core::Node;
using Core::NodePtr;
using Core::Project;
using Core::Property;
using Core::PropertyPtr;
using Editor::Modules::Timeline::KeyframeWidget;
using Editor::Modules::Timeline::KeyframeNodeWidget;
using Editor::Modules::Timeline::KeyframePropertyWidget;
using Editor::Modules::Timeline::KeyframeDelegate;
using Editor::Modules::Timeline::Model;
using ModelItemRoles = Model::ModelItemRoles;
using ModelItemDataType = Model::ModelItemDataType;

///

enum class WhichHandle { Start, Stop, Both };

using clicked_fn_t = std::function<void(bool)>;
using dragged_fn_t = std::function<void(WhichHandle, const int)>;
using released_fn_t = std::function<void()>;
using hover_fn_t = std::function<void(bool)>;

class DragHandle: public QWidget
{
public:
	DragHandle(QWidget* parent, WhichHandle which, dragged_fn_t draggedFn, released_fn_t releasedFn)
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

	WhichHandle which_;
	dragged_fn_t draggedFn_;
	released_fn_t releasedFn_;

	int dragX_;
	bool isDragging_ {};
};

class SelectionArea: public QWidget
{
public:
	SelectionArea(QWidget* parent, clicked_fn_t clickedFn, dragged_fn_t draggedFn, released_fn_t releasedFn, hover_fn_t hoverFn)
		: QWidget(parent)
		, clickedFn_(clickedFn)
		, draggedFn_(draggedFn)
		, releasedFn_(releasedFn)
		, hoverFn_(hoverFn)
	{
		setMouseTracking(true);
	}

private:
	void mousePressEvent(QMouseEvent* event) override
	{
		if (event->button() == Qt::LeftButton)
		{
			isDragging_ = true;
			isClicking_ = true;
			dragX_ = event->globalPos().x();
		}
	}

	void mouseMoveEvent(QMouseEvent* event) override
	{
		if (event->buttons() & Qt::LeftButton && isDragging_)
		{
			isClicking_ = false;
			int pos = event->globalPos().x() - dragX_;
			dragX_ = event->globalPos().x();
			draggedFn_(WhichHandle::Both, pos);
		}
	}

	void mouseReleaseEvent(QMouseEvent* event) override
	{
		if (event->button() == Qt::LeftButton)
		{
			if (isClicking_)
			{
				clickedFn_(event->modifiers() & Qt::ControlModifier);
			}
			else
			{
				releasedFn_();
			}
			isClicking_ = false;
			isDragging_ = false;
		}
	}

	void enterEvent(QEvent* event) override
	{
		hoverFn_(true);
	}

	void leaveEvent(QEvent* event) override
	{
		hoverFn_(false);
	}

	clicked_fn_t clickedFn_;
	dragged_fn_t draggedFn_;
	released_fn_t releasedFn_;
	hover_fn_t hoverFn_;

	int dragX_;
	bool isDragging_ {};
	bool isClicking_ {};
};

KeyframeWidget::KeyframeWidget(KeyframeDelegate& kd, Project& project, const Model& model, QWidget* parent)
	: QWidget(parent)
	, model_(model)
{}

KeyframeNodeWidget::KeyframeNodeWidget(KeyframeDelegate& kd, Project& project, const Model& model, QWidget* parent, NodePtr node)
	: KeyframeWidget(kd, project, model, parent)
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

	auto updateSelectionArea = [this](bool selected, bool hovering = false)
	{
		QString color = selected ? "888" : "666";
		if (hovering) color = "999";
		area_->setStyleSheet(QString("background-color: #" + color));
	};

	auto restrictOffset = [this, &project](Core::visibility_t offset, bool limitEquallyMinimal) -> auto
	{
		auto docVis = project.current().settings().visibility;

		Frame nStart = start_ + offset.first;
		Frame nStop = stop_ + offset.second;

		Frame dStart = 0, dStop = 0;

		if (nStart < docVis.first) dStart -= (nStart - docVis.first);
		if (nStop > docVis.second) dStop -= (nStop - docVis.second);

		if (limitEquallyMinimal)
		{
			if (-dStop > dStart) dStart = dStop;
			else if (dStart > -dStop) dStop = dStart;
		}

		offset.first += dStart;
		offset.second += dStop;

		return offset;
	};

	///

	auto clicked = [&](bool multiSelect) { emit kd.clicked(this, multiSelect); };

	auto dragged = [this, &kd, restrictOffset](WhichHandle which, const int offsetX)
	{
		Core::Frame offsetFrames = offsetX;
		Core::visibility_t offset, restricted;

		switch (which)
		{
		case WhichHandle::Start:
			offset = { offsetFrames, 0.0f };
			break;
		case WhichHandle::Stop:
			offset = { 0.0f, offsetFrames };
			break;
		case WhichHandle::Both:
			offset = { offsetFrames, offsetFrames };
			break;
		}
		
		restricted = restrictOffset(offset, which == WhichHandle::Both);
		emit kd.dragMoving(this, restricted);
	};

	auto released = [this, &kd]()
	{
		auto vis = node_->visibility();
		emit kd.dragEnded(this);
	};

	auto hovered = [this, &kd, updateSelectionArea](bool over)
	{
		updateSelectionArea(kd.isSelected(this), over);
	};

	///

	area_ = new SelectionArea(this, clicked, dragged, released, hovered);
	updateSelectionArea(kd.isSelected(this));
	area_->show();

	startHandle_ = new DragHandle(area_, WhichHandle::Start, dragged, released);
	startHandle_->setStyleSheet("background-color: #f0f;");
	stopHandle_ = new DragHandle(area_, WhichHandle::Stop, dragged, released);
	stopHandle_->setStyleSheet("background-color: #f0f;");
	startHandle_->show();
	stopHandle_->show();

	connect(&kd, &KeyframeDelegate::selectionChanged, this, [this, updateSelectionArea](KeyframeWidget* w, bool selected)
	{
		auto wn = qobject_cast<KeyframeNodeWidget*>(w);
		if (!wn || wn->node() != node_) return;
		updateSelectionArea(selected);
	});

	connect(&kd, &KeyframeDelegate::selectionMoved, this, [this, updateGeometry, restrictOffset](KeyframeWidget* w, Core::visibility_t offsets)
	{
		auto wn = qobject_cast<KeyframeNodeWidget*>(w);
		if (!wn || wn->node() != node_) return;
		offsets = restrictOffset(offsets, false);
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

KeyframePropertyWidget::KeyframePropertyWidget(KeyframeDelegate& kd, Project& project, const Model& model, QWidget* parent, PropertyPtr prop)
	: KeyframeWidget(kd, project, model, parent)
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

KeyframeDelegate::KeyframeDelegate(Project& project, const QSortFilterProxyModel& proxy, const Model& model)
	: project_(project)
	, proxy_(proxy)
	, model_(model)
{
}

QWidget* KeyframeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	auto type = static_cast<ModelItemDataType>(proxy_.data(index, static_cast<int>(ModelItemRoles::Type)).value<int>());

	KeyframeWidget* widget {};
	switch (type)
	{
	case ModelItemDataType::Node:
		widget = new KeyframeNodeWidget(const_cast<KeyframeDelegate&>(*this), project_, model_, parent, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<NodePtr>());
		break;
	case ModelItemDataType::Property:
		widget = new KeyframePropertyWidget(const_cast<KeyframeDelegate&>(*this), project_, model_, parent, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<PropertyPtr>());
		break;
	}

	widgets_.insert(widget);
	return widget;
}

void KeyframeDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
	widgets_.erase(qobject_cast<KeyframeWidget*>(editor));
	QStyledItemDelegate::destroyEditor(editor, index);
}

const KeyframeNodeWidget* KeyframeDelegate::findByNode(NodePtr node) const noexcept
{
	for (auto&& w : widgets_)
	{
		auto wn = qobject_cast<KeyframeNodeWidget*>(w);
		if (wn && wn->node() == node) return wn;
	}
	return nullptr;
}

void KeyframeDelegate::resetSelection()
{
	while (!selected_.empty()) setSelected(*begin(selected_), false);
}

void KeyframeDelegate::setSelected(KeyframeWidget* widget, bool selected)
{
	if (selected)
	{
		if (selected_.find(widget) != end(selected_)) return;
		selected_.insert(widget);
		emit selectionChanged(widget, true);
	}
	else
	{
		if (selected_.find(widget) == end(selected_)) return;
		selected_.erase(widget);
		emit selectionChanged(widget, false);
	}
}

bool KeyframeDelegate::isSelected(KeyframeWidget* widget) const
{
	return selected_.find(widget) != end(selected_);
}