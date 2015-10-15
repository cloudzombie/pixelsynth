#pragma once
#include <core/document.h>
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model: public QStandardItemModel
{
public:
	Model();

	QModelIndexList apply(std::shared_ptr<Core::MutationInfo> mutation, const QModelIndexList& oldSelection) noexcept;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
	QStandardItem* makeItem(const Core::NodePtr& node) const noexcept;
	int findChildIndex(QStandardItem* parent, QStandardItem* item) const noexcept;
	
	QStandardItem* findItem(const Core::Node& node) const noexcept;
	Core::Node* nodeFromIndex(const QModelIndex& index) const noexcept;
	QModelIndex indexFromNode(const Core::Node& node) const noexcept;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)