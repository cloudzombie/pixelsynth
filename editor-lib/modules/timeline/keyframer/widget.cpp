#include "widget.h"
#include "row_editor.h"

using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;

Widget::Widget(RowEditor* parent)
	: Draggable(parent)
	, editor_(parent)
{
}

void Widget::setSelected(bool selected)
{
	bool shouldEmit = selected_ != selected;
	selected_ = selected;
	update();
	if (shouldEmit) emit selectionChanged(selected);
}

void Widget::enterEvent(QEvent* event)
{
	hovering_ = true;
	update();
}

void Widget::leaveEvent(QEvent* event)
{
	hovering_ = false;
	update();
}

void Widget::paintEvent(QPaintEvent* pe)
{
	QStyleOption o;
	o.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
};
