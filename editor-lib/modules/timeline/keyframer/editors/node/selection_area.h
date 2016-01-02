#pragma once
#include <editor-lib/static.h>
#include "../../widget.h"

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

BEGIN_NAMESPACE(Keyframer) BEGIN_NAMESPACE(Editors) BEGIN_NAMESPACE(Node)

class SelectionArea: public Widget
{
	Q_OBJECT

public:
	SelectionArea(RowEditor* parent);

private:
	void paintEvent(QPaintEvent* event) override;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer) END_NAMESPACE(Editors) END_NAMESPACE(Node)