#include "property_editor.h"
#include "../delegate.h"
#include "../widget.h"
#include "../../model.h"
#include "property/key.h"

#include <core/project.h>

using Core::Document;
using Core::Project;
using Core::PropertyPtr;
using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::Keyframer::Delegate;
using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::Editors::PropertyEditor;
using Editor::Modules::Timeline::Keyframer::Editors::Property::Key;

PropertyEditor::PropertyEditor(Delegate& delegate, Project& project, const Model& model, QWidget* parent, PropertyPtr property)
	: RowEditor(parent)
	, property_(property)
{
	document_ = &project.current();

	///

	auto updateProperty = [&](PropertyPtr prevProperty, PropertyPtr curProperty)
	{
		if (prevProperty != property_) return;
		property_ = curProperty;
	};
	connect(&model, &Model::modelItemPropertyMutated, this, updateProperty);
	updateProperty(property_, property_);

	auto updateDocument = [this](const Document* prev, const Document* cur)
	{
		document_ = cur;
	};
	connect(&model, &Model::documentMutated, this, updateDocument);
	updateDocument(document_, document_);
}

const std::unordered_set<Widget*> PropertyEditor::widgets() const
{
	return { };
}

void PropertyEditor::applyMutation(Core::Document::Builder& mut)
{
}
