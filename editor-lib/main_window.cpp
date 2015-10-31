#include "main_window.h"
#include "modules/timeline/widget.h"

#include <editor-lib/application.h>

using Editor::Application;
using Editor::MainWindow;

MainWindow::MainWindow(Application& app)
	: app_(app)
{
	createMenu();

	timeline_ = new Modules::Timeline::Widget(this);
	setCentralWidget(timeline_);

	buttonDock_ = new QDockWidget(this);
	button_ = new QPushButton(buttonDock_);
	button_->setText("mutate");
	addDockWidget(Qt::TopDockWidgetArea, buttonDock_);

	connect(button_, &QPushButton::clicked, this, [&]()
	{
		static_cast<Modules::Timeline::Widget*>(timeline_)->mutate();
	});

	showMaximized();
}

void MainWindow::createMenu()
{
	// FILE
	auto actionNew = new QAction(tr("&New"), this);
	actionNew->setShortcuts(QKeySequence::New);
	connect(actionNew, &QAction::triggered, [&]() {
	});

	auto actionExit = new QAction(tr("E&xit"), this);
	actionExit->setShortcut(QKeySequence("Alt+X"));
	connect(actionExit, &QAction::triggered, [&]() { QCoreApplication::quit(); });

	auto fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(actionNew);
	fileMenu->addSeparator();
	fileMenu->addAction(actionExit);

	// EDIT
	actionUndo_ = new QAction(tr("&Undo"), this);
	actionUndo_->setEnabled(false);
	actionUndo_->setShortcuts(QKeySequence::Undo);
	connect(actionUndo_, &QAction::triggered, [&]() {
		app_.project().undo();
	});

	actionRedo_ = new QAction(tr("&Redo"), this);
	actionRedo_->setEnabled(false);
	actionRedo_->setShortcuts(QKeySequence::Redo);
	connect(actionRedo_, &QAction::triggered, [&]() {
		app_.project().redo();
	});

	auto editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(actionUndo_);
	editMenu->addAction(actionRedo_);

	connect(&app_, &Application::projectMutated, [&](auto mutationInfo)
	{
		auto undoState = app_.project().undoState();
		actionUndo_->setText(tr("&Undo") + " " + undoState.undoDescription.c_str());
		actionRedo_->setText(tr("&Redo") + " " + undoState.redoDescription.c_str());
		actionUndo_->setEnabled(undoState.canUndo);
		actionRedo_->setEnabled(undoState.canRedo);
	});
}