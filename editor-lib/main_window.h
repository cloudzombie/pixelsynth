#pragma once
#include "static.h"

BEGIN_NAMESPACE(Editor)

class MainWindow: public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

private:
	QWidget* timeline_;
	QDockWidget* buttonDock_;
	QPushButton* button_;
};

END_NAMESPACE(Editor)