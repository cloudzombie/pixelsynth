#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

BEGIN_NAMESPACE(Keyframer)

class Widget;
class RowEditor;

enum class TrimEdge { Start, Stop };

BEGIN_NAMESPACE(Editors)
class NodeEditor;
class PropertyEditor;
END_NAMESPACE(Editors)

class Delegate: public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit Delegate(Core::Project& project, const QSortFilterProxyModel& proxy, const Model& model);

	const std::unordered_set<RowEditor*> editors() const { return editors_; }

	void resetSelection();
	void setSelected(Widget* widget, bool selected);
	bool isSelected(Widget* widget) const;
	
	void deleteSelected();

	void setRubberBandSelection(QRect globalRect);

	Editors::NodeEditor* editorFor(Core::NodePtr node) const;
	Editors::PropertyEditor* editorFor(Core::PropertyPtr property) const;

public slots:
	void widgetCreated(Widget* widget);
	void widgetClicked(bool multiSelect);
	void widgetDragged(const int offset);
	void widgetTrimmed(const int offset, TrimEdge edge);
	void widgetReleased();

private:
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	const std::unordered_set<Widget*> widgets() const;
	const std::unordered_set<Widget*> selected() const;

	Core::Project& project_;
	const QSortFilterProxyModel& proxy_;
	const Model& model_;

	mutable std::unordered_set<RowEditor*> editors_;

	// The widgets that are selected by rubber band drag, but were not selected previously
	std::unordered_set<Widget*> dragSelected_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)