#include "property_value_item.h"
#include <editor-lib/event_bus.h>
#include <core/property.h>

using Editor::Modules::PropertyEditors::PropertyValueItem;

static QString doubleToString(double value, int decimals)
{
	QString str = QLocale::system().toString(value, 'f', decimals);
	if (qAbs(value) >= 1000.0) str.remove(QLocale::system().groupSeparator());
	return str;
}

///

struct PropertyValueAsEditRole
{
	template <typename T>
	QVariant operator()(const T& t)
	{
		return QVariant(t);
	}
};

///

struct PropertyValueAsDisplayRole
{
	template <typename T>
	QVariant operator()(const T& t)
	{
		return QVariant(t);
	}
};

///

struct EditRoleAsPropertyValue
{
	QVariant v;

	explicit EditRoleAsPropertyValue(QVariant v)
		: v(v)
	{}

	template <typename T>
	Core::PropertyValue operator()(const T& type)
	{
		return v.value<T>();
	}
};

template <> QVariant PropertyValueAsEditRole::operator()<glm::vec2>(const glm::vec2& t) { return QVariant(QVector2D(t.x, t.y)); }
template <> QVariant PropertyValueAsEditRole::operator()<glm::vec3>(const glm::vec3& t) { return QVariant(QVector3D(t.x, t.y, t.z)); }
template <> QVariant PropertyValueAsEditRole::operator()<std::string>(const std::string& t) { return QVariant(t.c_str()); }

template <> QVariant PropertyValueAsDisplayRole::operator()<glm::vec2>(const glm::vec2& t) { return QString("%1 %2").arg(doubleToString(t.x, 3)).arg(doubleToString(t.y, 3)); }
template <> QVariant PropertyValueAsDisplayRole::operator()<glm::vec3>(const glm::vec3& t) { return QString("%1 %2 %3").arg(doubleToString(t.x, 3)).arg(doubleToString(t.y, 3)).arg(doubleToString(t.z, 3)); }
template <> QVariant PropertyValueAsDisplayRole::operator()<std::string>(const std::string& t) { return t.c_str(); }

template <> Core::PropertyValue EditRoleAsPropertyValue::operator()<glm::vec2>(const glm::vec2& t) { return glm::vec2(v.value<QVector2D>().x(), v.value<QVector2D>().y()); }
template <> Core::PropertyValue EditRoleAsPropertyValue::operator()<glm::vec3>(const glm::vec3& t) { return glm::vec3(v.value<QVector3D>().x(), v.value<QVector3D>().y(), v.value<QVector3D>().z()); }
template <> Core::PropertyValue EditRoleAsPropertyValue::operator()<std::string>(const std::string& t) { return v.value<QString>().toStdString(); }

PropertyValueItem::PropertyValueItem(const Core::Property* prop)
	: prop_(prop)
{
	setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	LOG->debug("Creating PropertyValueItem for {}", *prop);
}

void PropertyValueItem::update(const Core::Property* prop)
{
	prop_ = prop;
	emitDataChanged();
}

QVariant PropertyValueItem::data(int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		return apply(PropertyValueAsDisplayRole(), prop_->getPropertyValue(0));
	case Qt::EditRole:
		return apply(PropertyValueAsEditRole(), prop_->getPropertyValue(0));
	case Qt::SizeHintRole:
		return QSize(0, 24);
	default:
		return QStandardItem::data(role);
	}
}

Core::PropertyValue PropertyValueItem::roundTrip() const
{
	QVariant qv = apply(PropertyValueAsEditRole(), prop_->getPropertyValue(0));
	return apply(EditRoleAsPropertyValue(qv), prop_->getPropertyValue(0));
}

void PropertyValueItem::setData(const QVariant& value, int role)
{
	if (role != Qt::EditRole)
	{
		return QStandardItem::setData(value, role);
	}

	emit Editor::EventBus::instance().propertyChanged(prop_, apply(EditRoleAsPropertyValue(value), prop_->getPropertyValue(0)));
}