#pragma once
#include <editor-lib/static.h>
#include <core/document.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

BEGIN_NAMESPACE(Keyframer)

class Delegate;
class Widget;

class RowEditor: public QWidget
{
	Q_OBJECT

public:
	RowEditor(QWidget* parent);

	static RowEditor* makeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::NodePtr node);
	static RowEditor* makeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::PropertyPtr property);

	virtual const std::unordered_set<Widget*> widgets() const = 0;
	virtual void applyMutation(Core::Document::Builder& mut) = 0;

	virtual Core::NodePtr node() const { return nullptr; }
	virtual Core::PropertyPtr property() const { return nullptr; }

protected:
	void paintEvent(QPaintEvent* pe) override;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)