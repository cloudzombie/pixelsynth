#include "draggable.h"

using Editor::Modules::Timeline::Keyframer::Draggable;

Draggable::Draggable(QWidget* parent)
	: QWidget(parent)
{
	setMouseTracking(true);
}

void Draggable::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		isDragging_ = true;
		isClicking_ = true;
		dragX_ = event->globalPos().x();
	}
}

void Draggable::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton && isDragging_)
	{
		isClicking_ = false;
		int offset = event->globalPos().x() - dragX_;
		dragX_ = event->globalPos().x();
		emit dragged(offset);
	}
}

void Draggable::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (isClicking_)
		{
			emit clicked(event->modifiers() & Qt::ControlModifier);
		}
		else
		{
			emit released();
		}
		isClicking_ = false;
		isDragging_ = false;
	}
}