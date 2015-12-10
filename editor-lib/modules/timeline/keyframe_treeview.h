#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;
class KeyframeWidget;
class KeyframeDelegate;

class KeyframeTreeView: public QTreeView
{
	Q_OBJECT

public:
	explicit KeyframeTreeView(Core::Project& project, QSortFilterProxyModel& proxy, Model& model, QWidget* parent);

private:
	void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	QRubberBand* rubberBand_;
	KeyframeDelegate* delegate_;

	QPoint dragPos_;
	bool isDragging_ {};

	// The widgets that are selected by rubber band drag, but were not selected previously
	std::unordered_set<KeyframeWidget*> dragSelected_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)