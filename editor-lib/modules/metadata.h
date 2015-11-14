#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules)

struct Metadata
{
	// Creates a widget for (1) app, with (2) parent widget and (3) the project, returns (4) widget
	using create_widget_t = std::function<QWidget*(QObject*, QWidget*, Core::Project&)>;

	// Creates an action for (1) app, related to (4) widget
	template <typename T>
	using create_action_t = std::function<QAction*(QObject*, T*)>;

	using action_list_t = std::vector<std::pair<create_action_t<QWidget>, QString>>;

	void setWidget(create_widget_t fn)
	{
		createWidget_ = fn;
	}

	void setDockWidgetArea(Qt::DockWidgetArea dockWidgetArea)
	{
		dockWidgetArea_ = dockWidgetArea;
	}

	template <typename T>
	void addAction(create_action_t<T> fn, QString before)
	{
		auto wrappedFn = [=](QObject* app, QWidget* widget)
		{
			return fn(app, qobject_cast<T*>(widget));
		};
		actions_.emplace_back(std::make_pair(wrappedFn, before));
	}

	create_widget_t createWidget() const { return createWidget_; }
	Qt::DockWidgetArea dockWidgetArea() const { return dockWidgetArea_; }
	action_list_t actions() const { return actions_; }

private:
	create_widget_t createWidget_ {};
	Qt::DockWidgetArea dockWidgetArea_ { Qt::BottomDockWidgetArea };
	action_list_t actions_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules)