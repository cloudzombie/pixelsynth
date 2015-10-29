#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(PropertyEditors)

class Delegate: public QStyledItemDelegate
{
	Q_OBJECT
		
public:
	Delegate(QObject* parent);
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(PropertyEditors)
