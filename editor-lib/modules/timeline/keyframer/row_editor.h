#pragma once
#include <editor-lib/static.h>
#include <core/document.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

BEGIN_NAMESPACE(Keyframer)

enum class TrimEdge;
class Delegate;
class Widget;
class ParentArea;

class RowEditor: public QWidget
{
	Q_OBJECT

public:
	RowEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent);

	static RowEditor* makeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::NodePtr node);
	static RowEditor* makeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::PropertyPtr property);

	virtual void initializeWidgets() = 0;

	virtual const std::unordered_set<Widget*> widgets() const = 0;
	virtual void applyMutations(Core::Document::Builder& mut, std::unordered_set<Widget*> selected) {};

	signals:
	void widgetCreated(Widget* widget) const;

protected:
	void paintEvent(QPaintEvent* pe) override;

	void updateDocument(const Core::Document* prev, const Core::Document* cur);
	void updateParentGeometry(RowEditor* parentEditor);

	Delegate& delegate_;
	const Core::Document* document_;

	ParentArea* parentArea_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)