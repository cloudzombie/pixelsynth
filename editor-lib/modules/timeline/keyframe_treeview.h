#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;
class KeyframeSelectionModel;

class KeyframeTreeView: public QTreeView
{
	Q_OBJECT

public:
	explicit KeyframeTreeView(Core::Project& project, QSortFilterProxyModel& proxy, Model& model, QWidget* parent);

private:
	void mousePressEvent(QMouseEvent* event) override;

	std::shared_ptr<KeyframeSelectionModel> selectionModel_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)