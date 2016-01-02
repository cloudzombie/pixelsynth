#include "parent_area.h"
#include "row_editor.h"
#include "delegate.h"
#include "widget.h"

using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::ParentArea;

ParentArea::ParentArea(QWidget* parent)
	: QWidget(parent)
{
}

void ParentArea::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);

	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor(64, 64, 80, 128));
	painter.drawRect(event->rect());
}

void ParentArea::updateGeometry(RowEditor* parentEditor)
{
	if (parentEditor)
	{
		// Base our area on the first widget of the parent editor. We assume this is a NodeEditor and so this widget contains the selection area.
		const auto&& w = parentEditor->widgets();
		auto widgetIt = w.cbegin();
		if (widgetIt != cend(w))
		{
			move((*widgetIt)->pos());
			resize((*widgetIt)->size());
			show();
			return;
		}
	}

	hide();
}