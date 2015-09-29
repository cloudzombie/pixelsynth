#pragma once
#include "static.h"
#include "node.h"

BEGIN_NAMESPACE(Core)

class Factory
{
public:
	using node_metadata_t = std::map<HashValue, Metadata*>;

	static void registerNodeMetadataProvider(HashValue nodeType, Metadata* metadata) noexcept;
	static std::shared_ptr<Node::Builder> makeNode(HashValue nodeType) noexcept;
	static Metadata* metadata(HashValue nodeType) noexcept;

private:
	static node_metadata_t nodes_;
};

END_NAMESPACE(Core)

#define DefineNode(title) ::Core::Factory::registerNodeMetadataProvider(Core::hash(#title), title::metadata());