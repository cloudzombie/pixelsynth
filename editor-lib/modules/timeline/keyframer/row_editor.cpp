#include "row_editor.h"
#include "editors/node_editor.h"
#include "editors/property_editor.h"

using Core::Project;
using Core::NodePtr;
using Core::PropertyPtr;
using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::Keyframer::Delegate;
using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;

RowEditor::RowEditor(QWidget* parent)
	: QWidget(parent)
{
	setStyleSheet("background-color: #333; border-top: 1px solid #444; margin-top: 1px");
}

void RowEditor::paintEvent(QPaintEvent* pe)
{
	QStyleOption o;
	o.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
};

///

RowEditor* RowEditor::makeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::NodePtr node)
{
	return new Editors::NodeEditor(delegate, project, model, parent, node);
}

RowEditor* RowEditor::makeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::PropertyPtr property)
{
	return new Editors::PropertyEditor(delegate, project, model, parent, property);
}
