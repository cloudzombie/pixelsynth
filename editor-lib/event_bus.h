#pragma once
#include "static.h"

BEGIN_NAMESPACE(Editor)

class EventBus: public QObject
{
	Q_OBJECT

public:
	EventBus();

	static EventBus& instance();

signals:
	void nodeSelectionChanged(const Core::NodePtr node, const bool selected) const;
	void propertyChanged(const Core::Property* prop, Core::PropertyValue newValue) const;

private:
};

END_NAMESPACE(Editor)