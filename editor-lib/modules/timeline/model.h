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

	Model();

	QModelIndexList apply(std::shared_ptr<Core::MutationInfo> mutation, const QModelIndexList& oldSelection) noexcept;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	const Core::Node* nodeFromIndex(const QModelIndex& index) const noexcept;
	const Core::Property* propertyFromIndex(const QModelIndex& index) const noexcept;

	// Converts the property value on that index into a QVariant and back again, used for testing purposes
	Core::PropertyValue roundTripPropertyValueFromIndex(const QModelIndex& index) const noexcept;

	void emitPropertyChanged(const Core::Property* prop, Core::PropertyValue newValue) const noexcept;

signals:
	void propertyChanged(const Core::Property* prop, Core::PropertyValue newValue) const;
	void modelItemNodeMutated(const Core::Node* prevNode, const Core::Node* curNode) const;
	void modelItemPropertyMutated(const Core::Property* prevProp, const Core::Property* curProp) const;

private:
	static int findChildIndex(QStandardItem* parent, ModelItem* item) noexcept;

	ModelItem* findItem(const void* ptr) const noexcept;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)