#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class KeyframeSelectionModel: public QWidget
{
	Q_OBJECT

public:
	void reset();
	void setSelected(Core::NodePtr node, bool selected);

	bool isSelected(Core::NodePtr node) const;
	std::unordered_set<Core::NodePtr> nodes() const { return nodes_; }

signals:
	void selectionChanged(Core::NodePtr node, bool selected);

public slots:
	void nodeMutated(Core::NodePtr prevNode, Core::NodePtr curNode);

private:
	std::unordered_set<Core::NodePtr> nodes_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)