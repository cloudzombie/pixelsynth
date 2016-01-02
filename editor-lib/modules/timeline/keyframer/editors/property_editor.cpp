#include "property_editor.h"
#include "node_editor.h"
#include "../delegate.h"
#include "../widget.h"
#include "../../model.h"
#include "property/key.h"

#include <core/project.h>

using Core::Document;
using Core::Frame;
using Core::Project;
using Core::PropertyPtr;
using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::Keyframer::Delegate;
using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::TrimEdge;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::Editors::PropertyEditor;
using Editor::Modules::Timeline::Keyframer::Editors::Property::Key;

PropertyEditor::PropertyEditor(Delegate& delegate, Project& project, const Model& model, QWidget* parent, PropertyPtr property)
	: RowEditor(delegate, project, model, parent)
	, property_(property)
{
	connect(&model, &Model::modelItemPropertyMutated, this, &PropertyEditor::updateProperty);
}

void PropertyEditor::initializeWidgets()
{
	updateProperty(property_, property_);
}

const std::unordered_set<Widget*> PropertyEditor::widgets() const
{
	std::unordered_set<Widget*> result;
	for (const auto& key : keys_) result.insert(key);
	return result;
}

void PropertyEditor::applyOffset(Frame offset, std::unordered_set<Widget*>& alreadyProcessed, std::function<bool(Widget*)> pred)
{
	for (auto&& key : keys_)
	{
		if (pred(key))
		{
			if (std::find(std::begin(alreadyProcessed), std::end(alreadyProcessed), key) != end(alreadyProcessed)) continue;

			key->setFrame(key->frame() + offset);
			alreadyProcessed.insert(key);
		}
	}

	updateParentGeometry();
}

void PropertyEditor::updateProperty(PropertyPtr prevProperty, PropertyPtr curProperty)
{
	if (prevProperty != property_) return;
	property_ = curProperty;

	std::vector<bool> wasSelected;

	for (const auto& key : keys_)
	{
		wasSelected.push_back(key->isSelected());
		key->deleteLater();
	}
	keys_.clear();

	auto selectIt = begin(wasSelected);
	for (const auto& frame : property_->keys())
	{
		auto key = new Key(frame, property_->getPropertyValue(frame), this);

		auto selected = false;
		if (selectIt != end(wasSelected)) selected = *selectIt++;
		key->setSelected(selected);

		keys_.emplace_back(key);
		key->show();

		emit widgetCreated(key);
	}

	updateParentGeometry();
}

void PropertyEditor::updateParentGeometry()
{
	auto node = document_->parent(*property_);
	auto editor = delegate_.editorFor(node);
	RowEditor::updateParentGeometry(editor);
}

bool PropertyEditor::isDirty() const
{
	for (auto&& key : keys_)
	{
		if (key->frame() != key->originalFrame()) return true;
	}
	return false;
}

void PropertyEditor::applyMutations(Core::Node::Builder& builder)
{
	if (!isDirty()) return;

	LOG->info("mutating prop: " + property_->metadata().title());
	builder.mutateProperty(property_, [&](Core::Property::Builder& pb)
	{
		for (auto&& key : keys_)
		{
			pb.erase(key->originalFrame());
			pb.set(key->frame(), key->value());
		}
	});
}
