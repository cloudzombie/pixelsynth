#include "mutation-info.h"
#include "document.h"

using Core::Document;
using Core::MutationInfo;
using Core::ConnectorMetadataCollection;
using Core::ConnectorMetadataPtr;
using Core::Node;
using Core::NodePtr;
using Core::PropertyPtr;
using Core::node_eq_uuid;

template <typename TMutationInfo, typename EqPred, typename ExtractNodeContainerPair, typename Container>
std::unordered_map<NodePtr, TMutationInfo> generateMutationInfo(Container& prevNodes, Container& curNodes) noexcept
{
	std::unordered_map<NodePtr, TMutationInfo> result;

	for (auto& curNode: curNodes)
	{
		auto curNodeContainerPair = ExtractNodeContainerPair()(curNode);
		for (auto&& curValue : curNodeContainerPair.second)
		{
			bool found = false;
			for (auto&& prevNode : prevNodes)
			{
				auto prevNodeContainerPair = ExtractNodeContainerPair()(prevNode);

				auto prevValue = std::find_if(cbegin(prevNodeContainerPair.second), cend(prevNodeContainerPair.second), EqPred(curValue));
				if (prevValue != cend(prevNodeContainerPair.second))
				{
					// Same based on hash/uuid equality in both containers. Are the pointers different (i.e. has the item mutated?)
					if (curValue != *prevValue) result[prevNodeContainerPair.first].mutated.insert({ *prevValue, curValue });

					found = true;
					break;
				}
			}

			if (!found)
			{
				// Not in previous, so added
				result[curNodeContainerPair.first].added.insert(curValue);
			}
		}
	}

	for (auto& prevNode : prevNodes)
	{
		auto prevNodeContainerPair = ExtractNodeContainerPair()(prevNode);
		for (auto&& prevValue : prevNodeContainerPair.second)
		{
			bool found = false;
			for (auto&& curNode : curNodes)
			{
				auto curNodeContainerPair = ExtractNodeContainerPair()(curNode);
				auto curValue = std::find_if(cbegin(curNodeContainerPair.second), cend(curNodeContainerPair.second), EqPred(prevValue));
				if (curValue != cend(curNodeContainerPair.second))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				// Not in current, so removed
				result[prevNodeContainerPair.first].removed.insert(prevValue);
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

std::shared_ptr<MutationInfo> MutationInfo::compare(const Document& prev, const Document& cur) noexcept
{
	auto info = std::make_shared<MutationInfo>();

	// For every node, we want to iterate of a collection of properties/connectors in the previous and current version of the node and generate mutation info from that
	// When generating the mutation info for the nodes themselves, we extract a dummy collection containing just the node itself
	info->nodes = generateMutationInfo<NodeMutationInfo, node_eq_uuid, extract_node_containers>(prev.nodes(), cur.nodes());

	// Use the info->nodes list of nodes to restrict the amount of checking we need to do
	auto mutatedNodes = extractPrevCurNodes(info->nodes);
	info->properties = generateMutationInfo<PropertyMutationInfo, property_eq_hash, extract_property_containers>(mutatedNodes.first, mutatedNodes.second);
	info->connectorMetadata = generateMutationInfo<ConnectorMetadataMutationInfo, connector_metadata_eq_hash, extract_connector_metadata_containers>(mutatedNodes.first, mutatedNodes.second);

	return info;
}
