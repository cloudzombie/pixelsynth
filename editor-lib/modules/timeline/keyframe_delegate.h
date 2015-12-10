#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;
class KeyframeDelegate;

class KeyframeWidget: public QWidget
{
	Q_OBJECT

public:
	explicit KeyframeWidget(KeyframeDelegate& kd, Core::Project& project, const Model& model, QWidget* parent);

	virtual const QWidget* area() const = 0;

protected:
	const Model& model_;
};

class KeyframeNodeWidget: public KeyframeWidget
{
	Q_OBJECT

public:
	explicit KeyframeNodeWidget(KeyframeDelegate& kd, Core::Project& project, const Model& model, QWidget* parent, Core::NodePtr node);

	const QWidget* area() const override { return area_; }
	Core::NodePtr node() const { return node_; }
	Core::visibility_t visibility() const { return { start_, stop_ }; }

private:
	Core::NodePtr node_;

	QWidget* area_;
	QWidget* startHandle_;
	QWidget* stopHandle_;
	Core::Frame start_;
	Core::Frame stop_;
};

class KeyframePropertyWidget: public KeyframeWidget
{
	Q_OBJECT

public:
	explicit KeyframePropertyWidget(KeyframeDelegate& kd, Core::Project& project, const Model& model, QWidget* parent, Core::PropertyPtr prop);

	const QWidget* area() const override { return nullptr; }

private:
	Core::PropertyPtr prop_;
};

class KeyframeDelegate: public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit KeyframeDelegate(Core::Project& project, const QSortFilterProxyModel& proxy, const Model& model);

	const std::unordered_set<KeyframeWidget*> widgets() const { return widgets_; }
	const std::unordered_set<KeyframeWidget*> selected() const { return selected_; }

	const KeyframeNodeWidget* findByNode(Core::NodePtr node) const noexcept;

	void resetSelection();
	void setSelected(KeyframeWidget* widget, bool selected);
	bool isSelected(KeyframeWidget* widget) const;

signals:
	void selectionChanged(KeyframeWidget* widget, bool selected);
	void selectionMoved(KeyframeWidget* widget, const Core::visibility_t offsets);
	void clicked(KeyframeWidget* widget, bool multiSelect) const;
	void dragMoving(KeyframeWidget* widget, const Core::visibility_t offsets) const;
	void dragEnded(KeyframeWidget* widget) const;

private:
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void destroyEditor(QWidget* editor, const QModelIndex& index) const override;

	Core::Project& project_;
	const QSortFilterProxyModel& proxy_;
	const Model& model_;

	mutable std::unordered_set<KeyframeWidget*> widgets_;
	std::unordered_set<KeyframeWidget*> selected_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)