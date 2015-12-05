#pragma once

#include <core/static.h>

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

Q_DECLARE_METATYPE(Core::NodePtr)
Q_DECLARE_METATYPE(Core::PropertyPtr)

inline std::ostream& operator<<(std::ostream& out, const QModelIndex& n)
{
	if (n.parent() == QModelIndex()) out << "QModelIndex(" << n.row() << ", " << n.column() << ", " << (n.isValid() ? "valid" : "invalid") << ")";
	else out << "QModelIndex(" << n.row() << ", " << n.column() << ", " << n.parent() << ", " << (n.isValid() ? "valid" : "invalid") << ")";
	return out;
}
