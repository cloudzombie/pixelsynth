#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline) BEGIN_NAMESPACE(Keyframer)

class Draggable: public QWidget
{
	Q_OBJECT

public:
	Draggable(QWidget* parent);

signals:
	void clicked(bool multiSelect);
	void dragged(int offset);
	void released();

private:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	int dragX_;
	bool isDragging_ {};
	bool isClicking_ {};
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)