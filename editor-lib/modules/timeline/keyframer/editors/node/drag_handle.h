#pragma once
#include <editor-lib/static.h>
#include "../../draggable.h"

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline) BEGIN_NAMESPACE(Keyframer) BEGIN_NAMESPACE(Editors) BEGIN_NAMESPACE(Node)

class DragHandle: public Draggable
{
	Q_OBJECT

public:
	DragHandle(QWidget* parent);
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer) END_NAMESPACE(Editors) END_NAMESPACE(Node)