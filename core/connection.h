#pragma once
#include "static.h"
#include "metadata.h"
#include "node.h"

BEGIN_NAMESPACE(Core)

class Connection
{
public:
	using connection_t = std::tuple<NodePtr, ConnectorMetadataPtr, NodePtr, ConnectorMetadataPtr>;
	using mutable_connection_t = std::tuple<MutableNodePtr, MutableConnectorMetadataPtr, MutableNodePtr, MutableConnectorMetadataPtr>;

private:
	struct Impl
	{
		connection_t connection_;
	};

public:
	explicit Connection(connection_t connection);
	~Connection();

	Connection(const Connection& rhs);
	Connection& operator=(const Connection& rhs);

	Connection(Connection&& rhs);
	Connection& operator=(Connection&& rhs);

	const connection_t& connection() const noexcept;

	NodePtr outputNode() const;
	ConnectorMetadataPtr output() const;
	NodePtr inputNode() const;
	ConnectorMetadataPtr input() const;

private:
	friend class cereal::access;
	template<class Archive> void save(Archive& archive) const;
	template<class Archive>	void load(Archive& archive);

	Connection();
	std::unique_ptr<Impl> impl_;
};

struct connection_eq
{
	explicit connection_eq(ConnectionPtr compare_to): compare_to_(compare_to) { }
	bool operator()(ConnectionPtr c1) const { return c1->connection() == compare_to_->connection(); }
private:
	ConnectionPtr compare_to_;
};

END_NAMESPACE(Core)