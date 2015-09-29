#include "factory.h"

using Core::Factory;
using Core::Hash;
using Core::Node;
using Core::Metadata;

Factory::node_metadata_t Factory::nodes_;

void Factory::registerNodeMetadataProvider(Hash nodeType, Metadata* metadata) noexcept
{
	nodes_[nodeType] = metadata;
}

std::shared_ptr<Node::Builder> Factory::makeNode(Hash nodeType) noexcept
{
	return std::make_shared<Node::Builder>(nodeType);
}

Metadata* Factory::metadata(Hash nodeType) noexcept
{
	if (!nodeType) return nullptr; // root

	assert(nodes_.find(nodeType) != end(nodes_));
	return nodes_[nodeType];
}