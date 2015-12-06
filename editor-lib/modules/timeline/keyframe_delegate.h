#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

class KeyframeNodeWidget: public QWidget
{
public:
	explicit KeyframeNodeWidget(Core::Project& project, const Model& model, Core::NodePtr node, QWidget* parent);

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
	explicit KeyframeDelegate(Core::Project& project, const QSortFilterProxyModel& proxy, const Model& model);

private:
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	Core::Project& project_;
	const QSortFilterProxyModel& proxy_;
	const Model& model_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)