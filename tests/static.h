#pragma once

#include <bandit/bandit.h>
#include <tree/tree_util.h>

#include <core/factory.h>
#include <core/metadata.h>
#include <core/node.h>
#include <core/project.h>

inline Core::PropertyPtr prop(Core::NodePtr& node, const char* propertyTitle)
{
	auto prop = find_if(begin(node->properties()), end(node->properties()), Core::property_eq_hash(node->nodeType(), Core::hash(propertyTitle)));
	if (prop == end(node->properties())) return nullptr;
	return *prop;
}

template <typename T>
inline T prop(Core::NodePtr& node, const char* propertyTitle, Core::Frame frame)
{
	auto p = prop(node, propertyTitle);
	if (!p) return T();
	return p->get<T>(frame);
}

inline std::shared_ptr<const Core::Node> findNode(Core::Project& project, std::string nodeTitle)
{
	auto result = std::find_if(cbegin(project.current().nodes()), cend(project.current().nodes()), [nodeTitle](auto& node)
	{
		return prop<std::string>(node, "Title", 0) == nodeTitle;
	});
	if (result == cend(project.current().nodes())) return std::shared_ptr<const Core::Node>();
	return *result;
}

inline Core::ConnectorMetadataPtr connector(std::shared_ptr<const Core::Node> node, std::string connectorTitle)
{
	auto result = find_if(cbegin(node->connectorMetadata()), cend(node->connectorMetadata()), [connectorTitle](auto& metadata)
	{
		return metadata->hash() == Core::hash(connectorTitle.c_str());
	});
	if (result == cend(node->connectorMetadata())) return nullptr;
	return *result;
}

inline std::ostream& operator<<(std::ostream &strm, Core::NodePtr &node) {
	return strm << "Node(" << prop<std::string>(node, "Title", 0) << ")";
}