#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;
class KeyframeDelegate;
class KeyframeSelectionModel;

class KeyframeNodeWidget: public QWidget
{
	Q_OBJECT

public:
	explicit KeyframeNodeWidget(const KeyframeDelegate& kd, Core::Project& project, const Model& model, const KeyframeSelectionModel& selectionModel, Core::NodePtr node, QWidget* parent);

	const QWidget* area() const { return area_; }
	Core::NodePtr node() const { return node_; }
	Core::visibility_t visibility() const { return { start_, stop_ }; }

private:
	const Model& model_;
	Core::NodePtr node_;

	QWidget* area_;
	QWidget* startHandle_;
	QWidget* stopHandle_;
	Core::Frame start_;
	Core::Frame stop_;
};

class KeyframePropertyWidget: public QWidget
{
	Q_OBJECT

public:
	explicit KeyframePropertyWidget(const Model& model, Core::PropertyPtr prop, QWidget* parent);

private:
	const Model& model_;
	Core::PropertyPtr prop_;
};

class KeyframeDelegate: public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit KeyframeDelegate(Core::Project& project, const QSortFilterProxyModel& proxy, const Model& model, const KeyframeSelectionModel& selectionModel);

	const std::unordered_set<KeyframeNodeWidget*> nodes() const { return nodes_; }
	const std::unordered_set<KeyframePropertyWidget*> properties() const { return properties_; }

	const KeyframeNodeWidget* findByNode(Core::NodePtr node) const noexcept;

signals:
	void nodePressed(Core::NodePtr node, bool multiSelect) const;
	void nodeDragged(Core::NodePtr node, const Core::visibility_t offsets) const;
	void nodeReleased(Core::NodePtr node, bool multiSelect) const;

private:
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void destroyEditor(QWidget* editor, const QModelIndex& index) const override;

	Core::Project& project_;
	const QSortFilterProxyModel& proxy_;
	const Model& model_;
	const KeyframeSelectionModel& selectionModel_;

	mutable std::unordered_set<KeyframeNodeWidget*> nodes_;
	mutable std::unordered_set<KeyframePropertyWidget*> properties_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)