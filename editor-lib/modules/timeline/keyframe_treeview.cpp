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

	connect(keyframeDelegate, &KeyframeDelegate::nodeClicked, this, [&](NodePtr node, bool multiSelect)
	{
		bool selected = false;

		if (selectionModel_->isSelected(node))
		{
			// Was this the only selected node or are we replacing the selection?
			if (selectionModel_->nodes().size() == 1 || !multiSelect)
			{
				selectionModel_->reset();
				if (!multiSelect) selected = true;
			}
		}
		else
		{
			if (!multiSelect) selectionModel_->reset();
			selected = true;
		}
		selectionModel_->setSelected(node, selected);
	});

	connect(&model, &Model::modelItemNodeMutated, selectionModel_.get(), &KeyframeSelectionModel::nodeMutated);

	connect(keyframeDelegate, &KeyframeDelegate::visibilityOffset, this, [&](const std::pair<Frame, Frame> offset)
	{
		project.mutate([&](Document::Builder& mut)
		{
			for (auto&& node: selectionModel_->nodes())
			{
				mut.mutate(node, [&](auto& builder)
				{
					Frame start, stop;
					std::tie(start, stop) = node->visibility();
					builder.mutateVisibility({ start + offset.first, stop + offset.second });
				});
			}
		}, "Change visibility");
	});
}

void KeyframeTreeView::mousePressEvent(QMouseEvent* event)
{
	selectionModel_->reset();
}