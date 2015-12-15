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

namespace Node { class SelectionArea; class DragHandle; }

class NodeEditor: public RowEditor
{
public:
	NodeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::NodePtr node);

private:
	const std::unordered_set<Widget*> widgets() const override;
	void applyMutation(Core::Document::Builder& mut) override;

	void applyOffset(Widget* widget, Core::Frame offset) override;
	void applyTrim(Widget* widget, Core::Frame offset, TrimEdge edge) override;
	void updateNode(Core::NodePtr prevNode, Core::NodePtr curNode);
	void updateGeometry();

	Core::NodePtr node_;

	Node::SelectionArea* area_;
	Node::DragHandle* startHandle_;
	Node::DragHandle* stopHandle_;

	Core::Frame start_, stop_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer) END_NAMESPACE(Editors)