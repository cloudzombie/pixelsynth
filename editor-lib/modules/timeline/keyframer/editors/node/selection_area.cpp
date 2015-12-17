#include "selection_area.h"
#include "../../widget.h"

using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::Editors::Node::SelectionArea;

SelectionArea::SelectionArea(RowEditor* parent)
	: Widget(parent)
{
	updateColor(false);
}

void SelectionArea::enterEvent(QEvent* event)
{
	updateColor(true);
}

void SelectionArea::leaveEvent(QEvent* event)
{
	updateColor(false);
}

void SelectionArea::updateColor(bool hovering)
{
	QColor color = QColor(125, 125, 125);

	if (selected_) color = QColor(160, 160, 160);
	else
	{
		if (hovering) color = QColor(180, 180, 180);
	}

	setStyleSheet("background-color: " + color.name() + "; border-top: 1px solid " + color.lighter().name());
}

void SelectionArea::setSelected(bool selected)
{
	selected_ = selected;
	updateColor(false);
}