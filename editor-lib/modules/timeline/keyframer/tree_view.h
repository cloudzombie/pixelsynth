#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

BEGIN_NAMESPACE(Keyframer)

class Widget;
class Delegate;

class TreeView: public QTreeView
{
	Q_OBJECT

public:
	explicit TreeView(Core::Project& project, QSortFilterProxyModel& proxy, Model& model, QWidget* parent);

	void deleteSelected();

	QModelIndexList expanded() const;

private:
	void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	QRubberBand* rubberBand_;
	Delegate* delegate_;

	QPoint dragPos_;
	bool isDragging_ {};

	std::unordered_set<QStandardItem*> expanded_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)