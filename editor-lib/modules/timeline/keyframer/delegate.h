#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

BEGIN_NAMESPACE(Keyframer)

class Widget;
class RowEditor;

class Delegate: public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit Delegate(Core::Project& project, const QSortFilterProxyModel& proxy, const Model& model);

	const std::unordered_set<Widget*> widgets() const;
	const std::unordered_set<Widget*> selected() const;

	void resetSelection();
	void setSelected(Widget* widget, bool selected);
	bool isSelected(Widget* widget) const;

signals:
	void clicked(Widget* widget, bool multiSelect);

private:
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	Core::Project& project_;
	const QSortFilterProxyModel& proxy_;
	const Model& model_;

	mutable std::unordered_set<RowEditor*> editors_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)