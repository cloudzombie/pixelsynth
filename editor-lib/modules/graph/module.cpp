#include "module.h"
#include "widget.h"
#include "../metadata.h"

Editor::Modules::Metadata Editor::Modules::Graph::registerModule()
{
	Metadata m;

	m.setWidget([&](QObject* app, QWidget* parent, Core::Project& project)
	{
		auto widget = new Widget(parent, project);
		app->connect(app, SIGNAL(projectMutated(std::shared_ptr<Core::MutationInfo>)), widget, SLOT(projectMutated(std::shared_ptr<Core::MutationInfo>)));
		return widget;
	});

	m.setDockWidgetArea(Qt::NoDockWidgetArea);

	return m;
}
