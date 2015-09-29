#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)

class Node;
class ConnectorMetadata;

class Connection
{
	struct Impl;

public:
	using connection_t = std::tuple<NodePtr, ConnectorMetadataPtr, NodePtr, ConnectorMetadataPtr>;

	Connection(connection_t connection);
	~Connection();

	Connection(const Connection& rhs);
	Connection& operator=(const Connection& rhs);

	Connection(Connection&& rhs);
	Connection& operator=(Connection&& rhs);

	const connection_t& connection() const noexcept;

private:
	std::unique_ptr<Impl> impl_;
};

END_NAMESPACE(Core)