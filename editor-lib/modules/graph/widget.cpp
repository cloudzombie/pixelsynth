#include "widget.h"

#include <core/utils.h>
#include <core/mutation_info.h>

using Editor::Modules::Graph::Widget;

using namespace Core;

Widget::Widget(QWidget* parent, Project& project)
	: QWidget(parent)
	, project_(project)
{
}

void Widget::projectMutated(std::shared_ptr<MutationInfo> mutationInfo) const
{
}