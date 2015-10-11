#pragma once
#include <core/document.h>
#include <editor-lib/static.h>
#include <core/project.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

class Widget: public QWidget
{
	Q_OBJECT

public:
	using selection_t = std::deque<Core::Uuid>;

	Widget(QWidget* parent);
	void mutate();

	selection_t selectedNodes() const noexcept;
	void setSelection(selection_t selection) noexcept;

private:
	QTreeView* tree_;
	std::shared_ptr<Model> model_;
	Core::Project p;
	size_t mutationIndex {};
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)