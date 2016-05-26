#include "widget.h"
#include "model.h"

#include <core/utils.h>
#include <core/mutation_info.h>

#include "../../event_bus.h"

using Editor::Modules::Inspector::Widget;

using namespace Core;

Widget::Widget(QWidget* parent, Project& project)
	: QDockWidget(parent)
	, project_(project)
	, tree_(new QTreeView(this))
	, proxy_(new QSortFilterProxyModel(this))
	, model_(std::make_shared<Model>())
{
	setWindowTitle(tr("Inspector"));
	setMinimumSize(QSize(400, 0));
	setWidget(tree_);

	proxy_->setSourceModel(model_.get());
	tree_->setModel(proxy_);

	connect(&EventBus::instance(), &EventBus::nodeSelectionChanged, this, [this](NodePtr node, bool selected) {
		if (selected) model_->selectNode(node);
		else model_->deselectNode(node);
	});
}

void Widget::projectMutated(std::shared_ptr<MutationInfo> mutationInfo) const
{
}