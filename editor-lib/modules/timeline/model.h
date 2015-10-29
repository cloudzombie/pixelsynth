#pragma once
#include <core/document.h>
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model: public QStandardItemModel
{
	class ModelItem;

public:
	Model();

	QModelIndexList apply(std::shared_ptr<Core::MutationInfo> mutation, const QModelIndexList& oldSelection) noexcept;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	const Core::Node* nodeFromIndex(const QModelIndex& index) const noexcept;
	const Core::Property* propertyFromIndex(const QModelIndex& index) const noexcept;

	// Converts the property value on that index into a QVariant and back again, used for testing purposes
	Core::PropertyValue roundTripPropertyValueFromIndex(const QModelIndex& index) const noexcept;

private:
	static int findChildIndex(QStandardItem* parent, ModelItem* item) noexcept;

	ModelItem* findItem(const void* ptr) const noexcept;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)