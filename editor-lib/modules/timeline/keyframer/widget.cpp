#include "widget.h"

using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;

Widget::Widget(RowEditor* editor, QWidget* parent)
	: Draggable(parent)
	, editor_(editor)
{
}

void Widget::paintEvent(QPaintEvent* pe)
{
	QStyleOption o;
	o.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
};
