#include "mutation_info.h"
#include "document.h"

using Core::ConnectionPtr;
using Core::ConnectorMetadataCollection;
using Core::ConnectorMetadataPtr;
using Core::Document;
using Core::MutationInfo;
using Core::Node;
using Core::NodePtr;
using Core::PropertyPtr;
using Core::node_eq_uuid;

using ChangeType = MutationInfo::ChangeType;
template <typename T>
using Change = MutationInfo::Change<T>;
template <typename T>
using ChangeSet = MutationInfo::ChangeSet<T>;

template <typename Container>
NodePtr findNodeByUuid(const Container& c, const Core::Uuid& uuid)
{
	auto it = find_if(cbegin(c), cend(c), [&](auto& node) { return node->uuid() == uuid; });
	if (it != cend(c)) return *it;
	return nullptr;
}

void findRemovedNodes(const MutationInfo& i, std::vector<Change<NodePtr>>& changes)
{
	for (auto&& prevNode: i.prevNodes)
	{
		auto curNode = findNodeByUuid(i.curNodes, prevNode->uuid());
		if (curNode) continue;
		changes.emplace_back(Change<NodePtr>(prevNode, {}, ChangeType::Removed, i.prev.parent(*prevNode), {}, i.prev.childIndex(*prevNode), -1));
	}
}

void findAddedOrMutatedNodes(const MutationInfo& i, std::vector<Change<NodePtr>>& changes)
{
	for (auto&& curNode : i.curNodes)
	{
		auto prevNode = findNodeByUuid(i.prevNodes, curNode->uuid());
		if (!prevNode)
		{
			// added
			changes.emplace_back(Change<NodePtr>({}, curNode, ChangeType::Added, {}, i.cur.parent(*curNode), -1, i.cur.childIndex(*curNode)));
		}
		else
		{
			if (prevNode != curNode || i.prev.parent(*prevNode) != i.cur.parent(*curNode) || i.prev.childIndex(*prevNode) != i.cur.childIndex(*curNode))
			{
				// mutated
				changes.emplace_back(Change<NodePtr>(prevNode, curNode, ChangeType::Mutated, i.prev.parent(*prevNode), i.cur.parent(*curNode), i.prev.childIndex(*prevNode), i.cur.childIndex(*curNode)));
			}
		}
	}
}

void findRemovedConnections(const MutationInfo& i, std::vector<Change<ConnectionPtr>>& changes)
{
	for (auto&& prevConn : i.prev.connections())
	{
		auto curConnIt = find(cbegin(i.cur.connections()), cend(i.cur.connections()), prevConn);
		if (curConnIt != cend(i.cur.connections())) continue;
		changes.emplace_back(Change<ConnectionPtr>(prevConn, {}, ChangeType::Removed, {}, {}, -1, -1));
	}
}

void findAddedOrMutatedConnections(const MutationInfo& i, std::vector<Change<ConnectionPtr>>& changes)
{
	for (auto&& curConn : i.cur.connections())
	{
		auto prevConnIt = find(cbegin(i.prev.connections()), cend(i.prev.connections()), curConn);
		if (prevConnIt != cend(i.prev.connections())) continue;
		changes.emplace_back(Change<ConnectionPtr>({}, curConn, ChangeType::Added, {}, {}, -1, -1));
	}
}

template <typename ITEMPTR, typename EqFn, typename GetItemsFn>
void findRemovedItems(const MutationInfo& i, std::vector<Change<ITEMPTR>>& changes, GetItemsFn getItems)
{
	for (auto&& prevNode : i.prevNodes)
	{
		auto curNode = findNodeByUuid(i.curNodes, prevNode->uuid());
		for (auto&& prevItem : getItems(prevNode))
		{
			bool removed = false;

			if (curNode)
			{
				auto& curNodeItems = getItems(curNode);
				auto itemIt = find_if(cbegin(curNodeItems), cend(curNodeItems), [&](auto& item)
				{
					return EqFn(prevItem)(item);
				});
				removed = itemIt == cend(curNodeItems);
			}
			else
			{
				removed = true;
			}

			if (removed) changes.emplace_back(Change<ITEMPTR>(prevItem, {}, ChangeType::Removed, prevNode, {}, i.prev.childIndex(*prevItem), -1));
		}
	}
}

template <typename ITEMPTR, typename EqFn, typename GetItemsFn>
void findAddedOrMutatedItems(const MutationInfo& i, std::vector<Change<ITEMPTR>>& changes, GetItemsFn getItems)
{
	for (auto&& curNode : i.curNodes)
	{
		auto prevNode = findNodeByUuid(i.prevNodes, curNode->uuid());
		for (auto&& curItem : getItems(curNode))
		{
			bool added = false;
			ITEMPTR prevItem {};

			if (prevNode)
			{
				auto& prevNodeItems = getItems(prevNode);
				auto itemIt = find_if(cbegin(prevNodeItems), cend(prevNodeItems), [&](auto& item)
				{
					return EqFn(curItem)(item);
				});

				if (itemIt == cend(prevNodeItems)) added = true;
				else prevItem = *itemIt;
			}
			else
			{
				added = true;
			}

			if (added)
			{
				// added
				changes.emplace_back(Change<ITEMPTR>({}, curItem, ChangeType::Added, {}, curNode, -1, i.cur.childIndex(*curItem)));
			}
			else
			{
				if (prevItem != curItem)
				{
					// mutated
					changes.emplace_back(Change<ITEMPTR>(prevItem, curItem, ChangeType::Mutated, prevNode, curNode, i.prev.childIndex(*prevItem), i.cur.childIndex(*curItem)));
				}
			}
		}
	}
}

MutationInfo::MutationInfo(const Document& prev, const Document& cur)
	: prev(prev)
	, cur(cur)
{
	for (auto&& node : prev.nodes()) if (node != prev.root()) prevNodes.emplace_back(node);
	for (auto&& node : cur.nodes()) if (node != cur.root()) curNodes.emplace_back(node);

	findRemovedNodes(*this, nodes);
	findAddedOrMutatedNodes(*this, nodes);

	auto getProperties = [&](const NodePtr& n) { return n->properties(); };
	findRemovedItems<PropertyPtr, property_eq_hash>(*this, properties, getProperties);
	findAddedOrMutatedItems<PropertyPtr, property_eq_hash>(*this, properties, getProperties);

	auto getConnectors = [&](const NodePtr& n) { return n->connectorMetadata(); };
	findRemovedItems<ConnectorMetadataPtr, connector_metadata_eq_hash>(*this, connectors, getConnectors);
	findAddedOrMutatedItems<ConnectorMetadataPtr, connector_metadata_eq_hash>(*this, connectors, getConnectors);

	findRemovedConnections(*this, connections);
	findAddedOrMutatedConnections(*this, connections);
}
