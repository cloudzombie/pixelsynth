#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

class Widget: public QDockWidget
{
	Q_OBJECT

public:
	Widget(QWidget* parent, Core::Project& project);
	void mutate();

	const Model& model() const { return *model_.get(); }

public slots:
	void projectMutated(std::shared_ptr<Core::MutationInfo> mutationInfo) const;

private:
	Core::Project& project_;
	QTreeView* tree_;
	QSortFilterProxyModel* proxy_;
	std::shared_ptr<Model> model_;
	size_t mutationIndex {};
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)