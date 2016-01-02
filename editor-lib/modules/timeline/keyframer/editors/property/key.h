#pragma once
#include <editor-lib/static.h>
#include "../../widget.h"

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline) BEGIN_NAMESPACE(Keyframer) BEGIN_NAMESPACE(Editors) BEGIN_NAMESPACE(Property)

class Key: public Widget
{
	Q_OBJECT

public:
	Key(Core::Frame frame, Core::PropertyValue value, RowEditor* parent);

	void setFrame(Core::Frame frame);
	Core::Frame frame() const { return frame_; }
	Core::Frame originalFrame() const { return originalFrame_; }

	Core::PropertyValue value() const { return value_; }

private:
	void paintEvent(QPaintEvent* event) override;

	Core::Frame originalFrame_;
	Core::Frame frame_;
	Core::PropertyValue value_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer) END_NAMESPACE(Editors) END_NAMESPACE(Property)