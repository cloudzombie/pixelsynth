#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Inspector)

class Model: public QStandardItemModel
{
	struct Impl;
	class ModelItem;

	Q_OBJECT

public:
	Model();
	void selectNode(Core::NodePtr node);
	void deselectNode(Core::NodePtr node);

private:
	void update();

	std::shared_ptr<Impl> impl_;
	std::unordered_set<Core::NodePtr> nodes_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Inspector)
