#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline) BEGIN_NAMESPACE(Keyframer)

class RowEditor;

class ParentArea: public QWidget
{
	Q_OBJECT

public:
	ParentArea(QWidget* parent);

	void updateGeometry(RowEditor* parentEditor);

private:
	void paintEvent(QPaintEvent* event) override;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)