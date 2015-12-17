#include "widget.h"
#include "row_editor.h"

using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;

Widget::Widget(RowEditor* parent)
	: Draggable(parent)
	, editor_(parent)
{
}

void Widget::paintEvent(QPaintEvent* pe)
{
	QStyleOption o;
	o.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
};
