#include "module.h"
#include "widget.h"
#include "../metadata.h"

Editor::Modules::Metadata Editor::Modules::Timeline::registerModule()
{
	Metadata m;

	m.setWidget([&](QObject* app, QWidget* parent, Core::Project& project)
	{
		auto widget = new Widget(parent, project);
		app->connect(app, SIGNAL(projectMutated(std::shared_ptr<Core::MutationInfo>)), widget, SLOT(projectMutated(std::shared_ptr<Core::MutationInfo>)));
		return widget;
	});

	m.setDockWidgetArea(Qt::BottomDockWidgetArea);

	// Cut/copy/paste

	m.addAction<Widget>([&](QObject* app, Widget* widget)
	{
		auto action = new QAction(app);
		action->setSeparator(true);
		return action;
	}, "&Redo", ActionFlags::RequiresFocus);

	m.addAction<Widget>([&](QObject* app, Widget* widget)
	{
		auto action = new QAction(app->tr("&Cut"), app);
		action->setShortcuts({ QKeySequence(app->tr("Ctrl+C")) });
		return action;
	}, "&Redo", ActionFlags::RequiresFocus);

	// Debug stuff

	m.addAction<Widget>([&](QObject* app, Widget* widget)
	{
		auto action = new QAction(app);
		action->setSeparator(true);
		return action;
	}, "&Redo");

	m.addAction<Widget>([&](QObject* app, Widget* widget)
	{
		auto action = new QAction(app->tr("&Mutate"), app);
		action->setShortcuts({ QKeySequence(app->tr("Ctrl+M")) });
		action->connect(action, &QAction::triggered, widget, &Widget::mutate);
		return action;
	}, "&Redo");

	return m;
}
