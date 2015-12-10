#include "keyframe_selectionmodel.h"

using Editor::Modules::Timeline::KeyframeSelectionModel;

void KeyframeSelectionModel::reset()
{
	while (!nodes_.empty()) setSelected(*begin(nodes_), false);
}

void KeyframeSelectionModel::setSelected(Core::NodePtr node, bool selected)
{
	if (selected)
	{
		if (nodes_.find(node) != end(nodes_)) return;
		nodes_.insert(node);
		emit selectionChanged(node, true);
	}
	else
	{
		if (nodes_.find(node) == end(nodes_)) return;
		nodes_.erase(node);
		emit selectionChanged(node, false);
	}
}

bool KeyframeSelectionModel::isSelected(Core::NodePtr node) const
{
	return nodes_.find(node) != end(nodes_);
}

void KeyframeSelectionModel::nodeMutated(Core::NodePtr prevNode, Core::NodePtr curNode)
{
	if (prevNode)
	{
		auto& it = nodes_.find(prevNode);
		if (it != end(nodes_))
		{
			nodes_.erase(prevNode);
			if (curNode) nodes_.insert(curNode);
		}
	}
}