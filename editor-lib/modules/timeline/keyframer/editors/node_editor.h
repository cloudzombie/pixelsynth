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
	Q_OBJECT

public:
	NodeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::NodePtr node);

	Core::NodePtr node() const { return node_; }

	const std::unordered_set<Widget*> widgets() const override;
	Widget* widget() const { return *begin(widgets()); }

	bool isSelected() const;
	bool isDirty() const;

	void applyOffset(Core::Frame offset, std::unordered_set<Widget*>& alreadyProcessed);
	void applyTrim(Core::Frame offset, TrimEdge edge, std::unordered_set<Widget*>& alreadyProcessed);

	void applyMutations(Core::Document::Builder& mut);

private:
	void initializeWidgets() override;

	void updateNode(Core::NodePtr prevNode, Core::NodePtr curNode);
	void updateGeometry();

	Core::NodePtr node_;

	Node::SelectionArea* area_;
	Node::DragHandle* startHandle_;
	Node::DragHandle* stopHandle_;

	Core::Frame start_, stop_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer) END_NAMESPACE(Editors)