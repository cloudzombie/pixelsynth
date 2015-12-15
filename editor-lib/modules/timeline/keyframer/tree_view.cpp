#include <core/node.h>
#include <core/project.h>
#include "tree_view.h"
#include "delegate.h"
#include "header.h"
#include "widget.h"
#include "row_editor.h"
#include "../model.h"

using Core::Document;
using Core::Project;
using Core::NodePtr;
using Core::Frame;
using Editor::Modules::Timeline::Keyframer::Delegate;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::TreeView;
using Editor::Modules::Timeline::Model;

TreeView::TreeView(Project& project, QSortFilterProxyModel& proxy, Model& model, QWidget* parent)
	: QTreeView(parent)
	, rubberBand_(new QRubberBand(QRubberBand::Rectangle, this))
{
	setIndentation(0);
	setModel(&proxy);
	setSelectionMode(QAbstractItemView::NoSelection);

	delegate_ = new Delegate(project, proxy, model);
	setItemDelegateForColumn(static_cast<int>(Model::Columns::Item), delegate_);
	setHeader(new Header(model, this));

	connect(delegate_, &Delegate::clicked, this, [&](Widget* widget, bool multiSelect)
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

	connect(delegate_, &Delegate::moved, this, [&](Widget* widget, const Frame offset)
	{
		auto widgets = delegate_->selected();
		widgets.insert(widget);

		for (auto&& w : widgets)
		{
			w->editor()->applyOffset(w, offset);
		}
	});

	connect(delegate_, &Delegate::trimmed, this, [&](Widget* widget, const Frame offset, TrimEdge edge)
	{
		auto widgets = delegate_->selected();
		widgets.insert(widget);

		for (auto&& w : widgets)
		{
			w->editor()->applyTrim(w, offset, edge);
		}
	});

	/*connect(delegate_, &Delegate::dragEnded, this, [&](Widget* widget)
	{
		project.mutate([&](Document::Builder& mut)
		{
			auto widgets = delegate_->selected();
			widgets.insert(widget);

			// Apply the mutations in document order, not particularly efficient, meh
			for (auto&& node : project.current().nodes())
			{
				for (auto&& w : widgets)
				{
					if (w->editor()->node() == node) w->editor()->applyMutation(mut);
				}
			}
		}, "Change visibility");
	});*/
}

void TreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	//QTreeView::drawRow(painter, option, index);
}

void TreeView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		isDragging_ = true;
		dragPos_ = event->globalPos();
		if (!(event->modifiers() & Qt::ControlModifier)) delegate_->resetSelection();
	}
}

void TreeView::mouseMoveEvent(QMouseEvent* event)
{
	if (isDragging_)
	{
		rubberBand_->setGeometry(QRect(mapFromGlobal(dragPos_), mapFromGlobal(event->globalPos())).normalized());
		rubberBand_->show();

		/*auto globalRect = QRect(dragPos_, event->globalPos()).normalized();
		auto w = delegate_->widgets();
		LOG->info("total widgets: {}", w.size());
		for (auto&& widget : delegate_->widgets())
		{
			auto globalNodeRect = QRect(0, 0, widget->width(), widget->height()).translated(widget->parentWidget()->mapToGlobal(widget->pos()));
			LOG->info("rect: ({}, {}) -- ({}, {})", globalNodeRect.topLeft().x(), globalNodeRect.topLeft().y(), globalNodeRect.bottomRight().x(), globalNodeRect.bottomRight().y());

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
		}*/
	}
}

void TreeView::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		isDragging_ = false;
		rubberBand_->hide();
	}
}