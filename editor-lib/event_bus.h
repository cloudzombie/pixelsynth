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
	void nodeSelectionChanged(const Core::NodePtr node, const bool selected);

private:
};

END_NAMESPACE(Editor)