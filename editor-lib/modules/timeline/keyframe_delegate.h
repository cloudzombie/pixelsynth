#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

class KeyframeNodeWidget: public QWidget
{
public:
	explicit KeyframeNodeWidget(const Model& model, const Core::Node* node, QWidget* parent);

private:
	void mousePressEvent(QMouseEvent* event) override;

	const Model& model_;
	const Core::Node* node_;
};

class KeyframePropertyWidget: public QWidget
{
public:
	explicit KeyframePropertyWidget(const Model& model, const Core::Property* prop, QWidget* parent);

private:
	const Model& model_;
	const Core::Property* prop_;
};

class KeyframeDelegate: public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit KeyframeDelegate(const QSortFilterProxyModel& proxy, const Model& model);

private:
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	const QSortFilterProxyModel& proxy_;
	const Model& model_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)