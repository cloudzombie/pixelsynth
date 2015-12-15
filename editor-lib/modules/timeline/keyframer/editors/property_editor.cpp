#include "property_editor.h"
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
	updateProperty(property_, property_);
}

const std::unordered_set<Widget*> PropertyEditor::widgets() const
{
	std::unordered_set<Widget*> result;
	for (const auto& key : keys_) result.insert(key);
	return result;
}

void PropertyEditor::applyOffset(Widget* widget, Frame offset)
{
	auto key = qobject_cast<Key*>(widget);
	key->setFrame(key->frame() + offset);
}

void PropertyEditor::applyTrim(Widget* widget, Core::Frame offset, TrimEdge edge)
{}

void PropertyEditor::updateProperty(PropertyPtr prevProperty, PropertyPtr curProperty)
{
	if (prevProperty != property_) return;
	property_ = curProperty;

	for (const auto& key : keys_) delete key;
	keys_.clear();

	for (const auto& frame : property_->keys())
	{
		auto key = new Key(frame, this, this);
		connect(key, &Key::clicked, this, [this, key](bool multiSelect) { emit delegate_.clicked(key, multiSelect); });
		connect(key, &Key::dragged, this, [this, key](int offset) { emit delegate_.moved(key, offset); });
		connect(key, &Key::released, this, [this, key]() { emit delegate_.released(key); });
		keys_.insert(key);
	}
}


void PropertyEditor::applyMutation(Core::Document::Builder& mut)
{
}
