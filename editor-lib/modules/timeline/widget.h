#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

class Widget: public QDockWidget
{
	Q_OBJECT

public:
	Widget(QWidget* parent, Core::Project& project);

	void deleteSelected();

	void mutate();

	const Model& model() const { return *model_.get(); }

public slots:
	void projectMutated(std::shared_ptr<Core::MutationInfo> mutationInfo) const;

private:
	void updateItemWidgets(QStandardItem* parent) const;
	void syncVerticalScrollBars(int value) const;

	Core::Project& project_;
	QWidget* container_;
	QGridLayout* layout_;
	QTreeView* tree_;
	QSplitter* splitter_;
	QTreeView* keyframer_;
	QSortFilterProxyModel* proxy_;
	std::shared_ptr<Model> model_;
	size_t mutationIndex {};
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)