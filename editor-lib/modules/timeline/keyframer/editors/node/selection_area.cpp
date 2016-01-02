#include "selection_area.h"
#include "../node_editor.h"
#include "../../widget.h"

using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::Editors::NodeEditor;
using Editor::Modules::Timeline::Keyframer::Editors::Node::SelectionArea;

SelectionArea::SelectionArea(RowEditor* parent)
	: Widget(parent)
{
}

void SelectionArea::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);

	QColor color = QColor(125, 125, 125);

	if (isSelected()) color = QColor(160, 160, 160);
	else
	{
		if (isHovering()) color = QColor(180, 180, 180);
	}

	painter.setPen(Qt::NoPen);
	painter.setBrush(color);
	painter.drawRect(event->rect());

	painter.setPen(color.darker());
	painter.drawLine(event->rect().topLeft(), event->rect().topRight());

	painter.setPen(color.lighter());
	painter.drawLine(event->rect().adjusted(0, 1, 0, 1).topLeft(), event->rect().adjusted(0, 1, 0, 1).topRight());
}