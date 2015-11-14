#include "module.h"
#include "widget.h"
#include "../metadata.h"

namespace em = Editor::Modules;

std::unique_ptr<em::Metadata> em::Timeline::registerModule()
{
	return Metadata::Builder()
		.withCreateWidget([&](QWidget* parent) -> QWidget* { return new Widget(parent); })
		.withDockWidgetArea(Qt::BottomDockWidgetArea)
		//.withAction(actionMutate)
		.build();
}
