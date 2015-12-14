#pragma once
#include <editor-lib/static.h>
#include "../../widget.h"

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline) BEGIN_NAMESPACE(Keyframer) BEGIN_NAMESPACE(Editors) BEGIN_NAMESPACE(Property)

class Key: public Widget
{
	Q_OBJECT

public:
	Key(RowEditor* editor, QWidget* parent);

	void setSelected(bool selected) override;

private:
	void paintEvent(QPaintEvent* event) override;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer) END_NAMESPACE(Editors) END_NAMESPACE(Property)