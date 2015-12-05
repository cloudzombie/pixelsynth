#include "keyframe_delegate.h"
#include "model.h"

#include <core/project.h>

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

KeyframeNodeWidget::KeyframeNodeWidget(Project& project, const Model& model, const Document& document, NodePtr node, QWidget* parent)
	: QWidget(parent)
	, document_(&document)
	, model_(model)
	, node_(node)
{
	auto updateWidth = [this]()
	{
		setFixedWidth(stop_ - start_);
		move(start_, pos().y());
		startHandle_->move(0 - startHandle_->width() * 0.5, 0);
		stopHandle_->move(width() - stopHandle_->width() * 0.5, 0);
	};

	auto dragged = [this, updateWidth](DragHandle::Which which, const int offset)
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
		updateWidth();
	};

	auto released = [this, &project]()
	{
		/*project.mutate([&](auto& mut)
		{
			mut.mutate(node_, [&](auto& node)
			{
				node.mutateVisibility({ start_, stop_ });
			});
		});*/
	};

	startHandle_ = new DragHandle(this, DragHandle::Which::Start, dragged, released);
	startHandle_->setStyleSheet("background-color: #f0f;");
	stopHandle_ = new DragHandle(this, DragHandle::Which::Stop, dragged, released);
	stopHandle_->setStyleSheet("background-color: #f0f;");
	startHandle_->show();
	stopHandle_->show();

	auto update = [this, updateWidth](const Document* prev, const Document* cur)
	{
		document_ = cur;

		std::tie(start_, stop_) = cur->settings().visibility;
		updateWidth();
	};

	auto updateNode = [&](NodePtr prevNode, NodePtr curNode)
	{
		if (prevNode != node_) return;
		node_ = curNode;
	};
	connect(&model, &Model::documentMutated, this, update);
	connect(&model, &Model::modelItemNodeMutated, this, updateNode);

	setBackgroundRole(QPalette::Shadow);
	update(&document, &document);
}

///

KeyframePropertyWidget::KeyframePropertyWidget(const Model& model, const Document& document, PropertyPtr prop, QWidget* parent)
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

KeyframeDelegate::KeyframeDelegate(Project& project, const QSortFilterProxyModel& proxy, const Model& model)
	: project_(project)
	, proxy_(proxy)
	, model_(model)
	, document_(nullptr)
{
	connect(&model, &Model::documentMutated, this, [&](const Document* prev, const Document* cur)
	{
		document_ = cur;
	});
}

QWidget* KeyframeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	auto type = static_cast<ModelItemDataType>(proxy_.data(index, static_cast<int>(ModelItemRoles::Type)).value<int>());

	switch (type)
	{
	case ModelItemDataType::Node:
		return new KeyframeNodeWidget(project_, model_, *document_, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<NodePtr>(), parent);
	case ModelItemDataType::Property:
		return new KeyframePropertyWidget(model_, *document_, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<PropertyPtr>(), parent);
	}

	return nullptr;
}