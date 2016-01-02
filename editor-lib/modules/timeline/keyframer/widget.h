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

	void setSelected(bool selected);
	bool isSelected() const noexcept { return selected_; }
	
	bool isHovering() const noexcept { return hovering_; }

	RowEditor* editor() const noexcept { return editor_; }

signals:
	void trimmed(const int offset, const TrimEdge edge);

protected:
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void paintEvent(QPaintEvent* pe) override;

	bool selected_ {};
	bool hovering_ {};
	RowEditor* editor_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)
