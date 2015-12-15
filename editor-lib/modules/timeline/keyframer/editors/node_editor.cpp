#include "node_editor.h"
#include "../delegate.h"
#include "../widget.h"
#include "../../model.h"
#include "node/selection_area.h"
#include "node/drag_handle.h"

#include <core/project.h>

using Core::Document;
using Core::Frame;
using Core::Project;
using Core::NodePtr;
using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::Keyframer::Delegate;
using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::TrimEdge;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::Editors::NodeEditor;
using Editor::Modules::Timeline::Keyframer::Editors::Node::SelectionArea;
using Editor::Modules::Timeline::Keyframer::Editors::Node::DragHandle;

NodeEditor::NodeEditor(Delegate& delegate, Project& project, const Model& model, QWidget* parent, NodePtr node)
	: RowEditor(delegate, project, model, parent)
	, node_(node)
{
	area_ = new SelectionArea(this, this);
	area_->setSelected(delegate.isSelected(area_));
	area_->show();

	startHandle_ = new DragHandle(area_);
	stopHandle_ = new DragHandle(area_);
	startHandle_->show();
	stopHandle_->show();

	connect(area_, &SelectionArea::clicked, this, [&](bool multiSelect) { emit delegate.clicked(area_, multiSelect); });
	connect(area_, &SelectionArea::dragged, this, [&](int offset) { emit delegate.moved(area_, offset); });
	connect(area_, &SelectionArea::released, this, [&]() { emit delegate.released(area_); });

	connect(startHandle_, &DragHandle::dragged, this, [&](int offset) { emit delegate.trimmed(area_, offset, TrimEdge::Start); });
	connect(stopHandle_, &DragHandle::dragged, this, [&](int offset) { emit delegate.trimmed(area_, offset, TrimEdge::Stop); });

	///

	connect(&model, &Model::modelItemNodeMutated, this, &NodeEditor::updateNode);
	updateNode(node_, node_);
}

const std::unordered_set<Widget*> NodeEditor::widgets() const
{
	return { area_ };
}

void NodeEditor::applyOffset(Widget* widget, Frame offset)
{
	start_ += offset;
	stop_ += offset;
	updateGeometry();
}

void NodeEditor::applyTrim(Widget* widget, Core::Frame offset, TrimEdge edge)
{
	switch (edge)
	{
	case TrimEdge::Start:
		start_ += offset;
		break;
	case TrimEdge::Stop:
		stop_ += offset;
		break;
	}
	updateGeometry();
}

void NodeEditor::updateNode(NodePtr prevNode, NodePtr curNode)
{
	if (prevNode != node_) return;
	node_ = curNode;

	std::tie(start_, stop_) = node_->visibility();
	updateGeometry();
}

void NodeEditor::updateGeometry()
{
	area_->setFixedWidth(stop_ - start_);
	area_->move(start_, area_->pos().y());
	startHandle_->move(0 - startHandle_->width() * 0.5, 0);
	stopHandle_->move(area_->width() - stopHandle_->width() * 0.5, 0);
}

void NodeEditor::applyMutation(Core::Document::Builder& mut)
{
}
