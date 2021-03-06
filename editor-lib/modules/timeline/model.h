#pragma once
#include <core/document.h>
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model: public QStandardItemModel
{
	Q_OBJECT
	class ModelItem;

public:
	enum class ModelItemDataType { Node, Property };
	enum class ModelItemRoles
	{
		Data = Qt::UserRole,
		Type = Data + 1
	};
	enum class Columns
	{
		Item,
		Value
	};

	// Applies mutations and returns the items that are not valid anymore (i.e. removed)
	std::vector<QStandardItem*> apply(std::shared_ptr<Core::MutationInfo> mutation) noexcept;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	Core::NodePtr nodeFromIndex(const QModelIndex& index) const noexcept;
	Core::PropertyPtr propertyFromIndex(const QModelIndex& index) const noexcept;
	QModelIndex findItemIndex(Core::NodePtr ptr) const noexcept;
	
	QSet<QStandardItem*> indicesToItems(const QModelIndexList& indices) const noexcept;
	QModelIndexList itemsToIndices(const QSet<QStandardItem*> items) const noexcept;

	// Converts the property value on that index into a QVariant and back again, used for testing purposes
	Core::PropertyValue roundTripPropertyValueFromIndex(const QModelIndex& index) const noexcept;

signals:
	void documentMutated(const Core::Document* prevDocument, const Core::Document* curDocument) const;
	void modelItemNodeMutated(Core::NodePtr prevNode, Core::NodePtr curNode) const;
	void modelItemPropertyMutated(Core::PropertyPtr prevProp, Core::PropertyPtr curProp) const;

private:
	static int findChildIndex(QStandardItem* parent, ModelItem* item) noexcept;

	ModelItem* findItem(QVariant ptr) const noexcept;
	ModelItem* findItem(Core::NodePtr ptr) const noexcept;
	ModelItem* findItem(Core::PropertyPtr ptr) const noexcept;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)