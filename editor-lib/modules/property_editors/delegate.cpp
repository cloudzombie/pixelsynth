#include "delegate.h"
#include "editors.h"

using Editor::Modules::PropertyEditors::Delegate;

Delegate::Delegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
	QItemEditorFactory* factory = new QItemEditorFactory;

	QItemEditorCreatorBase* editorVec3 = new QStandardItemEditorCreator<EditorVec3>();
	factory->registerEditor(QVariant::Vector3D, editorVec3);

	setItemEditorFactory(factory);
}