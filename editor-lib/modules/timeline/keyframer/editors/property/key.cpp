#include "key.h"

using Core::Frame;
using Core::PropertyValue;
using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::Editors::Property::Key;

Key::Key(Frame frame, PropertyValue value, RowEditor* parent)
	: Widget(parent)
	, originalFrame_(frame)
	, value_(value)
{
	setFixedSize(24, 24);
	setFrame(frame);
}

void Key::setFrame(Frame frame)
{
	frame_ = frame;
	move(frame_ - (width() / 2), 0);
}

void Key::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	painter.setPen(Qt::NoPen);
	painter.setBrush(selected_ ? QColor(200, 200, 200) : QColor(160, 160, 160));

	auto center = QPoint(width() * 0.5f, height() * 0.5f);
	auto size = QPoint(width() * 0.25f, height() * 0.25f);
	painter.drawEllipse(center, size.x(), size.y());
}