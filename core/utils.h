#pragma once
#include "static.h"

#include "node.h"
#include "project.h"
#include "factory.h"

BEGIN_NAMESPACE(Core)

static std::shared_ptr<Node> makeNode(HashValue type, std::string title)
{
	auto builder = Factory::makeNode(type);
	builder->mutateProperty(hash("$Title"), [&](auto& prop) { prop.set(0, title); });
	return std::make_shared<Node>(std::move(*builder));
}

inline PropertyPtr prop(const NodePtr& node, const char* propertyTitle)
{
	auto prop = find_if(begin(node->properties()), end(node->properties()), property_eq_hash(node->nodeType(), hash(propertyTitle)));
	if (prop == end(node->properties())) return nullptr;
	return *prop;
}

inline size_t propIndex(const NodePtr& node, const char* propertyTitle)
{
	auto prop = find_if(begin(node->properties()), end(node->properties()), property_eq_hash(node->nodeType(), hash(propertyTitle)));
	return distance(begin(node->properties()), prop);
}

template <typename T>
inline T prop(const NodePtr& node, const char* propertyTitle, Frame frame)
{
	auto p = prop(node, propertyTitle);
	if (!p) return T();
	return p->get<T>(frame);
}

inline std::shared_ptr<const Node> findNode(Project& project, std::string nodeTitle)
{
	auto result = std::find_if(cbegin(project.current().nodes()), cend(project.current().nodes()), [nodeTitle](auto& node)
	{
		return prop<std::string>(node, "$Title", 0) == nodeTitle;
	});
	if (result == cend(project.current().nodes())) return std::shared_ptr<const Node>();
	return *result;
}

inline ConnectorMetadataPtr connector(std::shared_ptr<const Node> node, std::string connectorTitle)
{
	auto result = find_if(cbegin(node->connectorMetadata()), cend(node->connectorMetadata()), [connectorTitle](auto& metadata)
	{
		return metadata->hash() == hash(connectorTitle.c_str());
	});
	if (result == cend(node->connectorMetadata())) return nullptr;
	return *result;
}

inline size_t connectorIndex(std::shared_ptr<const Node> node, std::string connectorTitle)
{
	auto result = find_if(cbegin(node->connectorMetadata()), cend(node->connectorMetadata()), [connectorTitle](auto& metadata)
	{
		return metadata->hash() == hash(connectorTitle.c_str());
	});
	return distance(cbegin(node->connectorMetadata()), result);
}

END_NAMESPACE(Core)