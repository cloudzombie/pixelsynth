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
{
	setIndentation(0);
	setModel(&proxy);
	setSelectionMode(QAbstractItemView::NoSelection);

	auto keyframeDelegate = new KeyframeDelegate(project, proxy, model, *selectionModel_.get());
	setItemDelegateForColumn(static_cast<int>(Model::Columns::Item), keyframeDelegate);
	setHeader(new KeyframeHeader(model, this));

	connect(keyframeDelegate, &KeyframeDelegate::nodePressed, this, [&](NodePtr node, bool multiSelect)
	{
		if (!multiSelect) selectionModel_->setSelected(node, true);
	});

	connect(keyframeDelegate, &KeyframeDelegate::nodeDragged, this, [&](const std::pair<Core::Frame, Core::Frame> offsets)
	{
		for (auto&& node : selectionModel_->nodes())
		{
			emit selectionModel_->selectionMoved(node, offsets);
		}
	});

	connect(keyframeDelegate, &KeyframeDelegate::nodeReleased, this, [&](NodePtr node, bool multiSelect, const std::pair<Frame, Frame> offset)
	{
		bool didDrag = fabs(offset.first) > 0.1 || fabs(offset.second) > 0.1;

		if (!didDrag)
		{
			// If we didn't drag this was just a regular click
			bool isSelected = selectionModel_->isSelected(node);

			if (!multiSelect)
			{
				selectionModel_->reset();
				selectionModel_->setSelected(node, true);
			}
			else
			{
				selectionModel_->setSelected(node, !isSelected);
			}
		}
		else
		{
			// We dragged!
			project.mutate([&](Document::Builder& mut)
			{
				for (auto&& node : selectionModel_->nodes())
				{
					mut.mutate(node, [&](auto& builder)
					{
						Frame start, stop;
						std::tie(start, stop) = node->visibility();
						builder.mutateVisibility({ start + offset.first, stop + offset.second });
					});
				}
			}, "Change visibility");
		}
	});

	connect(&model, &Model::modelItemNodeMutated, selectionModel_.get(), &KeyframeSelectionModel::nodeMutated);
}

void KeyframeTreeView::mousePressEvent(QMouseEvent* event)
{
	selectionModel_->reset();
}