#include "key.h"

using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::Editors::Property::Key;

Key::Key(RowEditor* editor, QWidget* parent)
	: Widget(editor, parent)
{
	setFixedSize(24, 24);
}

void Key::setSelected(bool selected)
{
	selected_ = selected;
	update();
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
