#pragma once

#include <functional>
#include <map>
#include <utility>
#include <memory>
#include <iostream>
#include <vector>
#include <stack>
#include <eggs/variant.hpp>
#include <tree/tree.h>

#include "hash.h"

#define BEGIN_NAMESPACE(name) namespace name {
#define END_NAMESPACE(name) }

namespace cereal
{
	class access;
}

namespace Core
{
	using Frame = float;
	using PropertyValue = eggs::variant<int, double, std::string>;

	enum class ConnectorType { Input, Output };

	class ConnectorMetadata;
	using ConnectorMetadataPtr = std::shared_ptr<const ConnectorMetadata>;
	using MutableConnectorMetadataPtr = std::shared_ptr<ConnectorMetadata>;
	using ConnectorMetadataCollection = std::vector<ConnectorMetadataPtr>;

	class PropertyMetadata;
	using PropertyMetadataPtr = std::shared_ptr<const PropertyMetadata>;
	using PropertyMetadataCollection = std::vector<PropertyMetadataPtr>;

	class Node;
	using NodePtr = std::shared_ptr<const Node>;
	using MutableNodePtr = std::shared_ptr<Node>;

	class Connection;
	using ConnectionPtr = std::shared_ptr<const Connection>;
	using MutableConnectionPtr = std::shared_ptr<Connection>;

	class Property;
	using PropertyPtr = std::shared_ptr<const Property>;
	using MutablePropertyPtr = std::shared_ptr<Property>;

	class Project;
	class Document;
};