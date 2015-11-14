#pragma once
#include "static.h"

BEGIN_NAMESPACE(Editor)

struct Actions
{
	QAction* newFile;
	QAction* openFile;
	QAction* saveFileAs;
	QAction* exit;

	QAction* undo;
	QAction* redo;

	explicit Actions(QObject* app);
};

END_NAMESPACE(Editor)
