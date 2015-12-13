#pragma once
#include <editor-lib/static.h>
#include <core/document.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;
class KeyframeDelegate;
class KeyframeWidget;
class PropertyKey;

enum class WhichHandle { Start, Stop, Both };

class KeyframeEditor: public QWidget
{
	Q_OBJECT

public:
	KeyframeEditor(QWidget* parent);

	virtual const std::unordered_set<KeyframeWidget*> widgets() const = 0;
	virtual void applyMutation(Core::Document::Builder& mut) = 0;

	virtual Core::NodePtr node() const { return nullptr; }
	virtual Core::PropertyPtr property() const { return nullptr; }

protected:
	void paintEvent(QPaintEvent* pe) override;
};

class KeyframeWidget: public QWidget
{
	Q_OBJECT

public:
	KeyframeWidget(KeyframeEditor* editor, QWidget* parent);

	virtual void setSelected(bool selected) = 0;
	bool isSelected() const noexcept { return selected_; }

	KeyframeEditor* editor() const noexcept { return editor_; }

protected:
	void paintEvent(QPaintEvent* pe) override;

	bool selected_ {};
	KeyframeEditor* editor_;
};

class KeyframeNodeEditor: public KeyframeEditor
{
	Q_OBJECT

public:
	explicit KeyframeNodeEditor(KeyframeDelegate& kd, Core::Project& project, const Model& model, QWidget* parent, Core::NodePtr node);

	const std::unordered_set<KeyframeWidget*> widgets() const override { return { area_ }; }
	Core::NodePtr node() const override { return node_; }

private:
	void applyMutation(Core::Document::Builder& mut);

	const Core::Document* document_;
	Core::NodePtr node_;

	KeyframeWidget* area_;
	QWidget* startHandle_;
	QWidget* stopHandle_;
	Core::Frame start_;
	Core::Frame stop_;
	WhichHandle dragType_;
};

class KeyframePropertyEditor: public KeyframeEditor
{
	Q_OBJECT

public:
	explicit KeyframePropertyEditor(KeyframeDelegate& kd, Core::Project& project, const Model& model, QWidget* parent, Core::PropertyPtr prop);

	const std::unordered_set<KeyframeWidget*> widgets() const override { return keys_; }
	void applyMutation(Core::Document::Builder& mut) {}
	Core::PropertyPtr property() const override { return prop_; }

private:
	Core::PropertyPtr prop_;
	QWidget* propertyArea_;
	std::unordered_set<KeyframeWidget*> keys_;
};

class KeyframeDelegate: public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit KeyframeDelegate(Core::Project& project, const QSortFilterProxyModel& proxy, const Model& model);

	const std::unordered_set<KeyframeWidget*> widgets() const;
	const std::unordered_set<KeyframeWidget*> selected() const;

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

	Core::Project& project_;
	const QSortFilterProxyModel& proxy_;
	const Model& model_;

	mutable std::unordered_set<KeyframeEditor*> editors_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)