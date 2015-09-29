#pragma once
#include "static.h"
#include "node.h"

BEGIN_NAMESPACE(Core)

class Factory
{
public:
	using node_metadata_t = std::map<Hash, Metadata*>;

	static void registerNodeMetadataProvider(Hash nodeType, Metadata* metadata) noexcept;
	static std::shared_ptr<Node::Builder> makeNode(Hash nodeType) noexcept;
	static Metadata* metadata(Hash nodeType) noexcept;

private:
	static node_metadata_t nodes_;
};

END_NAMESPACE(Core)

#define DefineNode(title) ::Core::Factory::registerNodeMetadataProvider(Core::hash(#title), title::metadata());