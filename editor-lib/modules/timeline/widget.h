#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

class Widget: public QDockWidget
{
	Q_OBJECT

public:
	Widget(QWidget* parent);
	void mutate();

private:
	QTreeView* tree_;
	std::shared_ptr<Model> model_;
	size_t mutationIndex {};
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)