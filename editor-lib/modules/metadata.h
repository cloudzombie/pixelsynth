#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules)

class Metadata
{
	using create_widget_t = std::function<QWidget*(QWidget*)>;
	using action_list_t = std::vector<std::pair<QAction*, QAction*>>;

	struct Impl;

public:
	create_widget_t& createWidget() const;
	Qt::DockWidgetArea dockWidgetArea() const;
	action_list_t& actions() const;

	class Builder
	{
	public:
		Builder();
		Builder& withCreateWidget(create_widget_t fn);
		Builder& withDockWidgetArea(Qt::DockWidgetArea area);
		Builder& withAction(QAction* action, QAction* before);
		std::unique_ptr<Metadata> build() noexcept;

		std::shared_ptr<Impl> impl_;
		friend class Metadata;
	};

private:
	Metadata(Builder&& rhs);

	std::shared_ptr<Impl> impl_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules)