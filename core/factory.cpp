#include "factory.h"

using Core::Factory;
using Core::HashValue;
using Core::Node;
using Core::Metadata;

Factory::node_metadata_t Factory::nodes_;

void Factory::registerNodeMetadataProvider(HashValue nodeType, Metadata* metadata) noexcept
{
	nodes_[nodeType] = metadata;
}

std::shared_ptr<Node::Builder> Factory::makeNode(HashValue nodeType) noexcept
{
	return std::make_shared<Node::Builder>(nodeType);
}

Metadata* Factory::metadata(HashValue nodeType) noexcept
{
	if (!nodeType) return nullptr; // root

	assert(nodes_.find(nodeType) != end(nodes_));
	return nodes_[nodeType];
}