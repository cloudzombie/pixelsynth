#include "node_editor.h"
#include "../delegate.h"
#include "../widget.h"
#include "../../model.h"
#include "node/selection_area.h"
#include "node/drag_handle.h"

#include <core/project.h>

using Core::Document;
using Core::Project;
using Core::NodePtr;
using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::Keyframer::Delegate;
using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::Editors::NodeEditor;
using Editor::Modules::Timeline::Keyframer::Editors::Node::SelectionArea;
using Editor::Modules::Timeline::Keyframer::Editors::Node::DragHandle;

NodeEditor::NodeEditor(Delegate& delegate, Project& project, const Model& model, QWidget* parent, NodePtr node)
	: RowEditor(parent)
	, node_(node)
{
	document_ = &project.current();

	auto updateGeometry = [this, project]()
	{
		area_->setFixedWidth(stop_ - start_);
		area_->move(start_, area_->pos().y());
		startHandle_->move(0 - startHandle_->width() * 0.5, 0);
		stopHandle_->move(area_->width() - stopHandle_->width() * 0.5, 0);
	};

	area_ = new SelectionArea(this, this);
	area_->setSelected(delegate.isSelected(area_));
	area_->show();

	startHandle_ = new DragHandle(area_);
	stopHandle_ = new DragHandle(area_);
	startHandle_->show();
	stopHandle_->show();

	connect(area_, &SelectionArea::clicked, this, [&](bool multiSelect) { emit delegate.clicked(area_, multiSelect); });

	///

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
		document_ = cur;
	};
	connect(&model, &Model::documentMutated, this, updateDocument);
	updateDocument(document_, document_);
}

const std::unordered_set<Widget*> NodeEditor::widgets() const
{
	return { area_ };
}

void NodeEditor::applyMutation(Core::Document::Builder& mut)
{
}
