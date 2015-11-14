#include "actions.h"

Editor::Actions::Actions(QObject* app)
{
	newFile = new QAction(app->tr("&New"), app);
	newFile->setShortcuts(QKeySequence::New);

	openFile = new QAction(app->tr("&Open"), app);
	openFile->setShortcuts(QKeySequence::Open);

	saveFileAs = new QAction(app->tr("Save &As"), app);
	saveFileAs->setShortcuts(QKeySequence::SaveAs);

	exit = new QAction(app->tr("E&xit"), app);
	exit->setShortcuts(QKeySequence::Quit);

	undo = new QAction(app->tr("&Undo"), app);
	undo->setEnabled(false);
	undo->setShortcuts(QKeySequence::Undo);

	redo = new QAction(app->tr("&Redo"), app);
	redo->setEnabled(false);
	redo->setShortcuts(QKeySequence::Redo);
}