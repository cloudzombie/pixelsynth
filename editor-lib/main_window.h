#pragma once
#include "static.h"

BEGIN_NAMESPACE(Editor)

class Application;

class MainWindow: public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(Application& app);

private:
	void createMenu();

	Application& app_;

	QWidget* timeline_;
	QDockWidget* buttonDock_;
	QPushButton* button_;

	QAction* actionUndo_;
	QAction* actionRedo_;
};

END_NAMESPACE(Editor)