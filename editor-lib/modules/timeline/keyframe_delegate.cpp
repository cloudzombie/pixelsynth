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
using Editor::Modules::Timeline::KeyframeEditor;
using Editor::Modules::Timeline::KeyframeNodeEditor;
using Editor::Modules::Timeline::KeyframePropertyEditor;
using Editor::Modules::Timeline::KeyframeDelegate;
using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::PropertyKey;
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

class SelectionArea: public KeyframeWidget
{
public:
	SelectionArea(KeyframeEditor* editor, QWidget* parent, clicked_fn_t clickedFn, dragged_fn_t draggedFn, released_fn_t releasedFn)
		: KeyframeWidget(editor, parent)
		, clickedFn_(clickedFn)
		, draggedFn_(draggedFn)
		, releasedFn_(releasedFn)
	{
		setMouseTracking(true);
		updateColor(false);
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
		updateColor(true);
	}

	void leaveEvent(QEvent* event) override
	{
		updateColor(false);
	}

	void updateColor(bool hovering)
	{
		QColor color = QColor(125, 125, 125);

		if (selected_) color = QColor(160, 160, 160);
		else
		{
			if (hovering) color = QColor(180, 180, 180);
		}

		setStyleSheet("background-color: " + color.name() + "; border-top: 1px solid " + color.lighter().name());
	}

	void setSelected(bool selected) override
	{
		selected_ = selected;
		updateColor(false);
	}

	clicked_fn_t clickedFn_;
	dragged_fn_t draggedFn_;
	released_fn_t releasedFn_;

	int dragX_;
	bool isDragging_ {};
	bool isClicking_ {};
};

KeyframeWidget::KeyframeWidget(KeyframeEditor* editor, QWidget* parent)
	: QWidget(parent)
	, editor_(editor)
{
}

void KeyframeWidget::paintEvent(QPaintEvent* pe)
{
	QStyleOption o;
	o.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
};

///

KeyframeEditor::KeyframeEditor(QWidget* parent)
	: QWidget(parent)
{
	setStyleSheet("background-color: #333; border-top: 1px solid #444; margin-top: 1px");
}

void KeyframeEditor::paintEvent(QPaintEvent* pe)
{
	QStyleOption o;
	o.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
};


///

KeyframeNodeEditor::KeyframeNodeEditor(KeyframeDelegate& kd, Project& project, const Model& model, QWidget* parent, NodePtr node)
	: KeyframeEditor(parent)
	, node_(node)
{
	auto updateGeometry = [this]()
	{
		area_->setFixedWidth(stop_ - start_);
		area_->move(start_, area_->pos().y());
		startHandle_->move(0 - startHandle_->width() * 0.5, 0);
		stopHandle_->move(area_->width() - stopHandle_->width() * 0.5, 0);
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

	auto clicked = [&](bool multiSelect) { emit kd.clicked(area_, multiSelect); };

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
		emit kd.dragMoving(area_, restricted);
	};

	auto released = [this, &kd]()
	{
		auto vis = node_->visibility();
		emit kd.dragEnded(area_);
	};

	///

	area_ = new SelectionArea(this, this, clicked, dragged, released);
	area_->setSelected(kd.isSelected(area_));
	area_->show();

	startHandle_ = new DragHandle(area_, WhichHandle::Start, dragged, released);
	stopHandle_ = new DragHandle(area_, WhichHandle::Stop, dragged, released);
	startHandle_->show();
	stopHandle_->show();

	///

	connect(&kd, &KeyframeDelegate::selectionChanged, this, [this](KeyframeWidget* w, bool selected)
	{
		if (w != area_) return;
		area_->setSelected(selected);
	});

	connect(&kd, &KeyframeDelegate::selectionMoved, this, [this, updateGeometry, restrictOffset](KeyframeWidget* w, Core::visibility_t offsets)
	{
		if (w != area_) return;
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

void KeyframeNodeEditor::applyMutation(Core::Document::Builder& mut)
{
	mut.mutate(node_, [&](auto& builder)
	{
		builder.mutateVisibility({ start_, stop_ });
	});
};

///

class PropertyKey: public KeyframeWidget
{
public:
	PropertyKey(KeyframeEditor* editor, QWidget* parent)
		: KeyframeWidget(editor, parent)
	{
		setFixedSize(24, 24);
		setMouseTracking(true);
	}

private:
	void setSelected(bool selected) override
	{
		selected_ = selected;
		update();
	}

	void paintEvent(QPaintEvent* event) override
	{
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		
		painter.setPen(Qt::NoPen);
		painter.setBrush(selected_ ? QColor(200, 200, 200) : QColor(160, 160, 160));

		auto center = QPoint(width() * 0.5f, height() * 0.5f);
		auto size = QPoint(width() * 0.25f, height() * 0.25f);
		painter.drawEllipse(center, size.x(), size.y());
	}
};

KeyframePropertyEditor::KeyframePropertyEditor(KeyframeDelegate& kd, Project& project, const Model& model, QWidget* parent, PropertyPtr prop)
	: KeyframeEditor(parent)
	, prop_(prop)
	, propertyArea_(new QWidget(this))
{
	propertyArea_->setStyleSheet("background-color: #445");

	auto update = [=](PropertyPtr prevProperty, PropertyPtr curProperty)
	{
		if (prevProperty != prop_) return;
		prop_ = curProperty;

		for (auto&& keyWidget : keys_) keyWidget->deleteLater();
		keys_.clear();

		auto node = project.current().parent(*prop_);
		if (!node) return;

		for (auto&& frame : curProperty->keys())
		{
			auto keyWidget = new PropertyKey(this, propertyArea_);
			keyWidget->move(frame - keyWidget->width() / 2, 0);
			keyWidget->show();
			keys_.insert(keyWidget);
		}
	};
	connect(&model, &Model::modelItemPropertyMutated, this, update);

	///

	connect(&kd, &KeyframeDelegate::selectionChanged, this, [this](KeyframeWidget* w, bool selected)
	{
		auto it = keys_.find(w);
		if (it == end(keys_)) return;
		(*it)->setSelected(selected);
	});

	///

	auto updateDocument = [this, update](const Document* prev, const Document* cur)
	{
		auto vis = cur->settings().visibility;
		setFixedWidth(vis.second - vis.first);
		propertyArea_->setFixedWidth(vis.second - vis.first);

		// PropertyArea should encompass node, except when there is no node visibility
		auto node = cur->parent(*prop_);
		if (node) vis = node->visibility();
		propertyArea_->setFixedWidth(vis.second - vis.first);
		propertyArea_->move(vis.first, propertyArea_->pos().y());

		update(prop_, prop_);
	};
	connect(&model, &Model::documentMutated, this, updateDocument);
	updateDocument(&project.current(), &project.current());
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

	KeyframeEditor* editor;
	switch (type)
	{
	case ModelItemDataType::Node:
	{
		editor = new KeyframeNodeEditor(const_cast<KeyframeDelegate&>(*this), project_, model_, parent, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<NodePtr>());
		break;
	}
	case ModelItemDataType::Property:
	{
		editor = new KeyframePropertyEditor(const_cast<KeyframeDelegate&>(*this), project_, model_, parent, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<PropertyPtr>());
		break;
	}
	}

	editors_.insert(editor);
	return editor;
}

void KeyframeDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
	editors_.erase(qobject_cast<KeyframeEditor*>(editor));
	QStyledItemDelegate::destroyEditor(editor, index);
}

void KeyframeDelegate::resetSelection()
{
	for (auto&& widget : selected()) emit selectionChanged(widget, false);
}

void KeyframeDelegate::setSelected(KeyframeWidget* widget, bool selected)
{
	auto s = this->selected();
	if (selected)
	{
		if (s.find(widget) != end(s)) return;
		emit selectionChanged(widget, true);
	}
	else
	{
		if (s.find(widget) == end(s)) return;
		emit selectionChanged(widget, false);
	}
}

bool KeyframeDelegate::isSelected(KeyframeWidget* widget) const
{
	auto s = selected();
	return s.find(widget) != end(s);
}

const std::unordered_set<KeyframeWidget*> KeyframeDelegate::widgets() const
{
	std::unordered_set<KeyframeWidget*> result;
	for (auto&& editor : editors_)
	{
		for (auto&& widget : editor->widgets()) result.insert(widget);
	}
	return result;
}

const std::unordered_set<KeyframeWidget*> KeyframeDelegate::selected() const
{
	std::unordered_set<KeyframeWidget*> result;
	for (auto&& editor : editors_)
	{
		for (auto&& widget : editor->widgets())
		{
			if (widget->isSelected()) result.insert(widget);
		}
	}
	return result;
}
