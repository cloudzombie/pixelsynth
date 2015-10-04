#pragma once
#include <core/document.h>
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model: public QAbstractItemModel
{
public:
	Model();

	void apply(std::shared_ptr<Core::MutationInfo> mutation);

	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& child) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
	Core::NodePtr fromId(quintptr id) const noexcept;
	quintptr toId(const Core::NodePtr& ptr) const noexcept;

	const Core::Document* doc_ {};
	mutable std::unordered_map<quintptr, Core::NodePtr> ptrIds_ {};
	mutable quintptr id_ {};
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)