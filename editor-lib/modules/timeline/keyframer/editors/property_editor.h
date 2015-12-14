#pragma once
#include <editor-lib/static.h>
#include <core/document.h>
#include "../row_editor.h"

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

BEGIN_NAMESPACE(Keyframer)

class Delegate;
class Widget;

BEGIN_NAMESPACE(Editors)

namespace Property { class Key; }

class PropertyEditor: public RowEditor
{
public:
	PropertyEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::PropertyPtr property);

private:
	const std::unordered_set<Widget*> widgets() const override;
	void applyMutation(Core::Document::Builder& mut) override;

	Core::PropertyPtr property_;
	const Core::Document* document_;

	Core::Frame start_, stop_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer) END_NAMESPACE(Editors)