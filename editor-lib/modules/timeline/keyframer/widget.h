#pragma once
#include <editor-lib/static.h>
#include "draggable.h"

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline) BEGIN_NAMESPACE(Keyframer)

class RowEditor;
enum class TrimEdge;

class Widget: public Draggable
{
	Q_OBJECT

public:
	Widget(RowEditor* parent);

	virtual void setSelected(bool selected) = 0;
	bool isSelected() const noexcept { return selected_; }

	RowEditor* editor() const noexcept { return editor_; }

signals:
	void trimmed(const int offset, const TrimEdge edge);

protected:
	void paintEvent(QPaintEvent* pe) override;

	bool selected_ {};
	RowEditor* editor_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)
