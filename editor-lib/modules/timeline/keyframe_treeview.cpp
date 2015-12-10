#include <core/node.h>
#include <core/project.h>
#include "keyframe_treeview.h"
#include "keyframe_delegate.h"
#include "keyframe_header.h"
#include "keyframe_selectionmodel.h"
#include "model.h"

using Core::Document;
using Core::Project;
using Core::NodePtr;
using Core::Frame;
using Editor::Modules::Timeline::KeyframeTreeView;
using Editor::Modules::Timeline::KeyframeSelectionModel;
using Editor::Modules::Timeline::Model;

KeyframeTreeView::KeyframeTreeView(Project& project, QSortFilterProxyModel& proxy, Model& model, QWidget* parent)
	: QTreeView(parent)
	, selectionModel_(std::make_shared<KeyframeSelectionModel>())
	, rubberBand_(new QRubberBand(QRubberBand::Rectangle, this))
{
	setIndentation(0);
	setModel(&proxy);
	setSelectionMode(QAbstractItemView::NoSelection);

	delegate_ = new KeyframeDelegate(project, proxy, model, *selectionModel_.get());
	setItemDelegateForColumn(static_cast<int>(Model::Columns::Item), delegate_);
	setHeader(new KeyframeHeader(model, this));

	connect(delegate_, &KeyframeDelegate::nodePressed, this, [&](NodePtr node, bool multiSelect)
	{
		if (!multiSelect)
		{
			selectionModel_->reset();
			selectionModel_->setSelected(node, true);
		}
		else
		{
			selectionModel_->setSelected(node, !selectionModel_->isSelected(node));
		}
	});

	connect(delegate_, &KeyframeDelegate::nodeDragged, this, [&](NodePtr node, const Core::visibility_t offsets)
	{
		selectionModel_->setSelected(node, true);

		for (auto&& n : selectionModel_->nodes())
		{
			emit selectionModel_->selectionMoved(n, offsets);
		}
	});

	connect(delegate_, &KeyframeDelegate::nodeReleased, this, [&](NodePtr node, bool multiSelect)
	{
		auto newVis = delegate_->findByNode(node)->visibility();
		bool didDrag = fabs(newVis.first - node->visibility().first) > 0.1 || fabs(newVis.second - node->visibility().second) > 0.1;
		if (!didDrag) return;
		
		project.mutate([&](Document::Builder& mut)
		{
			auto nodes = selectionModel_->nodes();
			nodes.insert(node);

			for (auto&& n : nodes)
			{
				mut.mutate(n, [&](auto& builder)
				{
					builder.mutateVisibility(delegate_->findByNode(n)->visibility());
				});
			}
		}, "Change visibility");
	});

	connect(&model, &Model::modelItemNodeMutated, selectionModel_.get(), &KeyframeSelectionModel::nodeMutated);
}

void KeyframeTreeView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		isDragging_ = true;
		dragPos_ = event->globalPos();
		if (!(event->modifiers() & Qt::ControlModifier)) selectionModel_->reset();
	}
}

void KeyframeTreeView::mouseMoveEvent(QMouseEvent* event)
{
	if (isDragging_)
	{
		rubberBand_->setGeometry(QRect(mapFromGlobal(dragPos_), mapFromGlobal(event->globalPos())).normalized());
		rubberBand_->show();

		auto globalRect = QRect(dragPos_, event->globalPos()).normalized();
		for (auto&& nodeWidget : delegate_->nodes())
		{
			auto area = nodeWidget->area();
			auto globalNodeRect = QRect(0, 0, area->width(), area->height()).translated(area->parentWidget()->mapToGlobal(area->pos()));

			auto node = nodeWidget->node();
			if (globalRect.intersects(globalNodeRect))
			{
				if (!selectionModel_->isSelected(node))
				{
					nodesDragSelected_.insert(node);
					selectionModel_->setSelected(node, true);
				}
			}
			else
			{
				if (nodesDragSelected_.find(node) != end(nodesDragSelected_))
				{
					nodesDragSelected_.erase(node);
					selectionModel_->setSelected(node, false);
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