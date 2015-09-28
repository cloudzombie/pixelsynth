#include "factory.h"

using Core::Factory;
using Core::Hash;
using Core::Node;
using Core::Metadata;

Factory::node_metadata_t Factory::nodes_;

void Factory::registerNodeMetadataProvider(Hash hash, Metadata* metadata) noexcept
{
	nodes_[hash] = metadata;
}

std::shared_ptr<Node::Builder> Factory::makeNode(Hash hash) noexcept
{
	assert(nodes_.find(hash) != end(nodes_));
	return std::make_shared<Node::Builder>(nodes_[hash]);
}
