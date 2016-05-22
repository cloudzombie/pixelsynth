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
}

void TreeView::deleteSelected()
{
	delegate_->deleteSelected();
}

void TreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// Don't want to show the actual row, since we're showing an editor instead
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
		delegate_->setRubberBandSelection(QRect(dragPos_, event->globalPos()).normalized());
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