#include "widget.h"

#include <core/utils.h>
#include <core/mutation_info.h>

using Editor::Modules::Inspector::Widget;

using namespace Core;

Widget::Widget(QWidget* parent, Project& project)
	: QDockWidget(parent)
	, project_(project)
	, container_(new QWidget(this))
	, layout_(new QVBoxLayout())
{
	setWindowTitle(tr("Inspector"));
	setMinimumSize(QSize(400, 0));

	container_->setLayout(layout_);
	setWidget(container_);
}

void Widget::projectMutated(std::shared_ptr<MutationInfo> mutationInfo) const
{
}