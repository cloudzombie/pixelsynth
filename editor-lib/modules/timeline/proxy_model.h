#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class ProxyModel: public QSortFilterProxyModel
{
public:
	ProxyModel(QObject* parent);

private:
	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)