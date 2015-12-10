#include <core/node.h>
#include <core/project.h>
#include "keyframe_treeview.h"
#include "keyframe_delegate.h"
#include "keyframe_header.h"
#include "model.h"

using Core::Document;
using Core::Project;
using Core::NodePtr;
using Core::Frame;
using Editor::Modules::Timeline::KeyframeWidget;
using Editor::Modules::Timeline::KeyframeTreeView;
using Editor::Modules::Timeline::Model;

KeyframeTreeView::KeyframeTreeView(Project& project, QSortFilterProxyModel& proxy, Model& model, QWidget* parent)
	: QTreeView(parent)
	, rubberBand_(new QRubberBand(QRubberBand::Rectangle, this))
{
	setIndentation(0);
	setModel(&proxy);
	setSelectionMode(QAbstractItemView::NoSelection);

	delegate_ = new KeyframeDelegate(project, proxy, model);
	setItemDelegateForColumn(static_cast<int>(Model::Columns::Item), delegate_);
	setHeader(new KeyframeHeader(model, this));

	connect(delegate_, &KeyframeDelegate::clicked, this, [&](KeyframeWidget* widget, bool multiSelect)
	{
		if (!multiSelect)
		{
			delegate_->resetSelection();
			delegate_->setSelected(widget, true);
		}
		else
		{
			delegate_->setSelected(widget, !delegate_->isSelected(widget));
		}
	});

	connect(delegate_, &KeyframeDelegate::dragMoving, this, [&](KeyframeWidget* widget, const Core::visibility_t offsets)
	{
		auto widgets = delegate_->selected();
		widgets.insert(widget);

		for (auto&& w : widgets)
		{
			emit delegate_->selectionMoved(w, offsets);
		}
	});

	connect(delegate_, &KeyframeDelegate::dragEnded, this, [&](KeyframeWidget* widget)
	{
		auto wn = qobject_cast<KeyframeNodeWidget*>(widget);
		if (!wn) return;

		auto newVis = wn->visibility();
		bool didDrag = fabs(newVis.first - wn->node()->visibility().first) > 0.1 || fabs(newVis.second - wn->node()->visibility().second) > 0.1;
		if (!didDrag) return;
		
		project.mutate([&](Document::Builder& mut)
		{
			auto widgets = delegate_->selected();
			widgets.insert(widget);

			for (auto&& w : widgets)
			{
				auto wn = qobject_cast<KeyframeNodeWidget*>(w);
				if (!wn) continue;
				mut.mutate(wn->node(), [&](auto& builder)
				{
					builder.mutateVisibility(wn->visibility());
				});
			}
		}, "Change visibility");
	});
}

void KeyframeTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{}

void KeyframeTreeView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		isDragging_ = true;
		dragPos_ = event->globalPos();
		if (!(event->modifiers() & Qt::ControlModifier)) delegate_->resetSelection();
	}
}

void KeyframeTreeView::mouseMoveEvent(QMouseEvent* event)
{
	if (isDragging_)
	{
		rubberBand_->setGeometry(QRect(mapFromGlobal(dragPos_), mapFromGlobal(event->globalPos())).normalized());
		rubberBand_->show();

		auto globalRect = QRect(dragPos_, event->globalPos()).normalized();
		for (auto&& widget : delegate_->widgets())
		{
			auto area = widget->area();
			if (!area) continue;
			auto globalNodeRect = QRect(0, 0, area->width(), area->height()).translated(area->parentWidget()->mapToGlobal(area->pos()));

			if (globalRect.intersects(globalNodeRect))
			{
				if (!delegate_->isSelected(widget))
				{
					dragSelected_.insert(widget);
					delegate_->setSelected(widget, true);
				}
			}
			else
			{
				if (dragSelected_.find(widget) != end(dragSelected_))
				{
					dragSelected_.erase(widget);
					delegate_->setSelected(widget, false);
				}
			}
		}
	}
}

void KeyframeTreeView::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		isDragging_ = false;
		rubberBand_->hide();
	}
}