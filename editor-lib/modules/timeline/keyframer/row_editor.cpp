#include "row_editor.h"
#include "editors/node_editor.h"
#include "editors/property_editor.h"
#include "../model.h"
#include <core/project.h>
#include "parent_area.h"

using Core::Document;
using Core::Project;
using Core::NodePtr;
using Core::PropertyPtr;
using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::Keyframer::Delegate;
using Editor::Modules::Timeline::Keyframer::ParentArea;
using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;

RowEditor::RowEditor(Delegate& delegate, Project& project, const Model& model, QWidget* parent)
	: QWidget(parent)
	, delegate_(delegate)
	, document_(&project.current())
{
	parentArea_ = new ParentArea(this);

	setStyleSheet("background-color: #333; border-top: 1px solid #444; margin-top: 1px");

	connect(&model, &Model::documentMutated, this, &RowEditor::updateDocument);
	updateDocument(document_, document_);
}

void RowEditor::paintEvent(QPaintEvent* pe)
{
	QStyleOption o;
	o.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
};

void RowEditor::updateDocument(const Document* prev, const Document* cur)
{
	auto vis = cur->settings().visibility;
	setFixedWidth(vis.second - vis.first);
	document_ = cur;
}

void RowEditor::updateParentGeometry(RowEditor* parentEditor)
{
	// Update background area indicating parent
	parentArea_->updateGeometry(parentEditor);
}

///

RowEditor* RowEditor::makeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::NodePtr node)
{
	return new Editors::NodeEditor(delegate, project, model, parent, node);
}

RowEditor* RowEditor::makeEditor(Delegate& delegate, Core::Project& project, const Model& model, QWidget* parent, Core::PropertyPtr property)
{
	return new Editors::PropertyEditor(delegate, project, model, parent, property);
}
