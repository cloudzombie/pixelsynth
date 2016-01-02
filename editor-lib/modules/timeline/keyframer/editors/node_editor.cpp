#include "node_editor.h"
#include "../delegate.h"
#include "../widget.h"
#include "../../model.h"
#include "node/selection_area.h"
#include "node/drag_handle.h"
#include "property_editor.h"

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
	area_ = new SelectionArea(this);
	area_->setSelected(delegate.isSelected(area_));
	area_->show();

	startHandle_ = new DragHandle(area_);
	stopHandle_ = new DragHandle(area_);
	startHandle_->show();
	stopHandle_->show();

	connect(startHandle_, &DragHandle::dragged, this, [this](int offset) { emit area_->trimmed(offset, TrimEdge::Start); });
	connect(stopHandle_, &DragHandle::dragged, this, [this](int offset) { emit area_->trimmed(offset, TrimEdge::Stop); });
	connect(startHandle_, &DragHandle::released, this, [this]() { emit area_->released(); });
	connect(stopHandle_, &DragHandle::released, this, [this]() { emit area_->released(); });

	///

	connect(&model, &Model::modelItemNodeMutated, this, &NodeEditor::updateNode);
	updateNode(node_, node_);
}

void NodeEditor::initializeWidgets()
{
	emit widgetCreated(area_);
}

const std::unordered_set<Widget*> NodeEditor::widgets() const
{
	return { area_ };
}

void NodeEditor::applyOffset(Frame offset, std::unordered_set<Widget*>& alreadyProcessed)
{
	if (std::find(std::begin(alreadyProcessed), std::end(alreadyProcessed), area_) != end(alreadyProcessed)) return;
	alreadyProcessed.insert(area_);

	start_ += offset;
	stop_ += offset;
	updateGeometry();

	// Apply to properties
	for (auto&& prop : node_->properties())
	{
		auto propertyEditor = delegate_.editorFor(prop);
		propertyEditor->applyOffset(offset, alreadyProcessed, [](Widget* w) { return true; });
	}

	// Recursively apply to children
	for (size_t t = 0; t < document_->childCount(*node_); t++)
	{
		auto child = document_->child(*node_, t);
		auto ne = qobject_cast<NodeEditor*>(delegate_.editorFor(child));
		if (ne) ne->applyOffset(offset, alreadyProcessed);
	}
}

void NodeEditor::applyTrim(Core::Frame offset, TrimEdge edge, std::unordered_set<Widget*>& alreadyProcessed)
{
	if (std::find(std::begin(alreadyProcessed), std::end(alreadyProcessed), area_) != end(alreadyProcessed)) return;
	alreadyProcessed.insert(area_);

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

	// Recursively apply to children
	for (size_t t = 0; t < document_->childCount(*node_); t++)
	{
		auto child = document_->child(*node_, t);
		auto ne = qobject_cast<NodeEditor*>(delegate_.editorFor(child));
		if (ne) ne->applyTrim(offset, edge, alreadyProcessed);
	}
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

	updateParentGeometry(delegate_.editorFor(document_->parent(*node_)));

	// Apply to properties
	for (auto&& prop : node_->properties())
	{
		auto propertyEditor = delegate_.editorFor(prop);
		if (propertyEditor) propertyEditor->updateParentGeometry();
	}
}

bool NodeEditor::isSelected() const
{
	return area_->isSelected();
}

bool NodeEditor::isDirty() const
{
	auto startOffset = start_ - node_->visibility().first;
	auto stopOffset = stop_ - node_->visibility().second;

	if (fabs(startOffset) > 0.01 || fabs(stopOffset) > 0.01) return true;

	for (auto&& prop : node_->properties())
	{
		auto propertyEditor = delegate_.editorFor(prop);
		if (propertyEditor->isDirty()) return true;
	}
	return false;
}

void NodeEditor::applyMutations(Core::Document::Builder& mut)
{
	mut.mutate(node_, [&](Core::Node::Builder& builder)
	{
		builder.mutateVisibility({ start_, stop_ });

		for (auto&& prop : node_->properties())
		{
			auto propertyEditor = delegate_.editorFor(prop);
			propertyEditor->applyMutations(builder);
		}
	});
}