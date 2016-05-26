#include "event_bus.h"
#include "application.h"

using Editor::EventBus;

EventBus::EventBus()
{
}

EventBus& EventBus::instance()
{
	return static_cast<Application*>(QCoreApplication::instance())->eventBus();
}
