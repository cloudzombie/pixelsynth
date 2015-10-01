#include "mutation-info.h"
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

// In case the result is a map keyed off NodePtr
template <typename DestContainer, typename SrcContainer>
struct get_set_result_container
{
	DestContainer& operator()(SrcContainer& result, const NodePtr node) { return result[node]; }
};

// In case the result is just a flat container
template <typename DestContainer>
struct get_set_result_container<DestContainer, MutationInfo::ConnectionMutationInfo>
{
	DestContainer& operator()(MutationInfo::ConnectionMutationInfo& result, const ConnectionPtr) { return result; }
};

template <typename Result, typename MutationInfoType, typename EqPred, typename ExtractContainerPair, typename Container>
Result generateMutationInfo(Container& prevItems, Container& curItems) noexcept
{
	using GetResultContainer = get_set_result_container<MutationInfoType, Result>;

	Result result;

	for (auto& curItem: curItems)
	{
		auto curContainerPair = ExtractContainerPair()(curItem);
		for (auto&& curValue : curContainerPair.second)
		{
			bool found = false;
			for (auto&& prevItem : prevItems)
			{
				auto prevContainerPair = ExtractContainerPair()(prevItem);

				auto prevValue = std::find_if(cbegin(prevContainerPair.second), cend(prevContainerPair.second), EqPred(curValue));
				if (prevValue != cend(prevContainerPair.second))
				{
					// Same based on hash/uuid equality in both containers. Are the pointers different (i.e. has the item mutated?)
					if (curValue != *prevValue) GetResultContainer()(result, prevContainerPair.first).mutated.insert({ *prevValue, curValue });

					found = true;
					break;
				}
			}

			if (!found)
			{
				// Not in previous, so added
				GetResultContainer()(result, curContainerPair.first).added.insert(curValue);
			}
		}
	}

	for (auto& prevItem : prevItems)
	{
		auto prevContainerPair = ExtractContainerPair()(prevItem);
		for (auto&& prevValue : prevContainerPair.second)
		{
			bool found = false;
			for (auto&& curItem : curItems)
			{
				auto curContainerPair = ExtractContainerPair()(curItem);
				auto curValue = std::find_if(cbegin(curContainerPair.second), cend(curContainerPair.second), EqPred(prevValue));
				if (curValue != cend(curContainerPair.second))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				// Not in current, so removed
				GetResultContainer()(result, prevContainerPair.first).removed.insert(prevValue);
			}
		}
	}

	return result;
}

std::pair<std::vector<NodePtr>, std::vector<NodePtr>> extractPrevCurNodes(std::unordered_map<NodePtr, MutationInfo::NodeMutationInfo> map)
{
	std::pair<std::vector<NodePtr>, std::vector<NodePtr>> result;

	for (auto&& nodeKvp: map)
	{
		if (nodeKvp.second.added.size())
		{
			result.second.emplace_back(nodeKvp.first);
			continue;
		}

		if (nodeKvp.second.removed.size())
		{
			result.first.emplace_back(nodeKvp.first);
			continue;
		}

		for (auto&& item: nodeKvp.second.mutated)
		{
			result.first.emplace_back(nodeKvp.first);
			result.second.emplace_back(item.second);
		}
	}

	return result;
}

struct extract_node_containers
{
	std::pair<NodePtr, std::vector<NodePtr>> operator()(const NodePtr node) const { return { node, { node } }; }
};

struct extract_property_containers
{
	std::pair<NodePtr, Node::properties_t> operator()(const NodePtr node) const { return { node, node->properties() }; }
};

struct extract_connector_metadata_containers
{
	std::pair<NodePtr, ConnectorMetadataCollection> operator()(const NodePtr node) const { return { node, node->connectorMetadata() }; }
};

struct extract_connection_containers
{
	std::pair<ConnectionPtr, std::vector<ConnectionPtr>> operator()(const ConnectionPtr c) const { return { c, { c } }; }
};

MutationInfo::reparenting_t generateReparentingInfo(const Document& prev, const Document& cur) noexcept
{
	MutationInfo::reparenting_t result;

	for (auto&& prevNode: prev.nodes())
	{
		auto curNode = std::find_if(cbegin(cur.nodes()), cend(cur.nodes()), node_eq_uuid(prevNode));
		if (curNode == cend(cur.nodes())) continue;

		// Node already existed, so see if it got reparented
		auto prevParent = prev.parent(prevNode);
		auto curParent = cur.parent(*curNode);
		if (prevParent == curParent) continue;

		result[prevNode] = { prevParent, curParent };
	}
	return result;
}

std::shared_ptr<MutationInfo> MutationInfo::compare(const Document& prev, const Document& cur) noexcept
{
	auto info = std::make_shared<MutationInfo>();

	// For every node, we want to iterate of a collection of properties/connectors in the previous and current version of the node and generate mutation info from that
	// When generating the mutation info for the nodes themselves, we extract a dummy collection containing just the node itself
	info->nodes = generateMutationInfo<std::unordered_map<NodePtr, NodeMutationInfo>, NodeMutationInfo, node_eq_uuid, extract_node_containers>(prev.nodes(), cur.nodes());

	// Use the info->nodes list of nodes to restrict the amount of checking we need to do
	auto mutatedNodes = extractPrevCurNodes(info->nodes);
	info->properties = generateMutationInfo<std::unordered_map<NodePtr, PropertyMutationInfo>, PropertyMutationInfo, property_eq_hash, extract_property_containers>(mutatedNodes.first, mutatedNodes.second);
	info->connectorMetadata = generateMutationInfo<std::unordered_map<NodePtr, ConnectorMetadataMutationInfo>, ConnectorMetadataMutationInfo, connector_metadata_eq_hash, extract_connector_metadata_containers>(mutatedNodes.first, mutatedNodes.second);

	// Get connection mutations
	info->connections = generateMutationInfo<ConnectionMutationInfo, ConnectionMutationInfo, connection_eq, extract_connection_containers>(prev.connections(), cur.connections());

	// Get reparenting mutations
	info->reparenting = generateReparentingInfo(prev, cur);

	return info;
}
