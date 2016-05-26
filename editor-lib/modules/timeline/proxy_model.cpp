#include "proxy_model.h"
#include "model.h"

using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::ProxyModel;

ProxyModel::ProxyModel(QObject* parent)
	: QSortFilterProxyModel(parent)
{}

bool ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	auto rowIndex = sourceParent.child(sourceRow, 0);

	// Ignore any internal properties (title starts with '$')
	auto& prop = static_cast<Model*>(sourceModel())->propertyFromIndex(rowIndex);
	if (prop && prop->metadata().title().at(0) == '$') return false;

	return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}