#include "drag_handle.h"

using Editor::Modules::Timeline::Keyframer::Draggable;
using Editor::Modules::Timeline::Keyframer::Editors::Node::DragHandle;

DragHandle::DragHandle(QWidget* parent)
	: Draggable(parent)
{
	resize(5, parent->height());
	setCursor(Qt::SizeHorCursor);
	setMouseTracking(true);
}