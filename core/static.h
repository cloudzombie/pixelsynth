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

namespace Core
{
	using Frame = float;
	using PropertyValue = eggs::variant<int, double, std::string>;

	enum class ConnectorType { Input, Output };

	class ConnectorMetadata;
	using ConnectorMetadataCollection = std::vector<ConnectorMetadata>;

	class Node;
	using NodePtr = std::shared_ptr<const Node>;

	class Connection;
	using ConnectionPtr = std::shared_ptr<const Connection>;
};