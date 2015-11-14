#include "metadata.h"

using Editor::Modules::Metadata;

struct Metadata::Impl
{
	create_widget_t createWidget;
	Qt::DockWidgetArea dockWidgetArea;
	action_list_t actions;
};

Metadata::Metadata(Builder&& rhs)
	: impl_(move(rhs.impl_))
{
}

Metadata::create_widget_t& Metadata::createWidget() const {	return impl_->createWidget; }
Qt::DockWidgetArea Metadata::dockWidgetArea() const { return impl_->dockWidgetArea; }
Metadata::action_list_t& Metadata::actions() const { return impl_->actions; }

Metadata::Builder::Builder()
	: impl_(std::make_shared<Impl>())
{
}

Metadata::Builder& Metadata::Builder::withCreateWidget(create_widget_t fn)
{
	impl_->createWidget = fn;
	return *this;
}

Metadata::Builder& Metadata::Builder::withDockWidgetArea(Qt::DockWidgetArea area)
{
	impl_->dockWidgetArea = area;
	return *this;
}

Metadata::Builder& Metadata::Builder::withAction(QAction* action, QAction* before)
{
	impl_->actions.emplace_back(std::make_pair(action, before));
	return *this;
}

std::unique_ptr<Metadata> Metadata::Builder::build() noexcept
{
	return std::unique_ptr<Metadata>(new Metadata(std::move(*this)));
}