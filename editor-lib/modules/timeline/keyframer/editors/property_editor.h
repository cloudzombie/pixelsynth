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
	Q_OBJECT

public:
	PropertyEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::PropertyPtr property);

	Core::PropertyPtr property() const { return property_; }

	bool isDirty() const;

	void applyMutations(Core::Node::Builder& builder);

	void applyOffset(Core::Frame offset, std::unordered_set<Widget*>& alreadyProcessed, std::function<bool(Widget*)> pred);
	void applySelectionOffset(Core::Frame offset, std::unordered_set<Widget*>& alreadyProcessed);

	void updateParentGeometry();

private:
	void initializeWidgets() override;

	const std::unordered_set<Widget*> widgets() const override;

	void updateProperty(Core::PropertyPtr prevProperty, Core::PropertyPtr curProperty);
	void updateDocument(const Core::Document* prev, const Core::Document* cur);

	Core::PropertyPtr property_;

	Core::Frame start_, stop_;
	std::vector<Property::Key*> keys_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer) END_NAMESPACE(Editors)