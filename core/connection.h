#pragma once
#include "static.h"

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

	template<class Archive>
	void save(Archive& archive) const
	{
		archive(impl_->connection_);
	}

	template<class Archive>
	void load(Archive& archive)
	{
		mutable_connection_t loaded;
		archive(loaded);

		NodePtr outputNode;
		ConnectorMetadataPtr output;
		NodePtr inputNode;
		ConnectorMetadataPtr input;
		tie(outputNode, output, inputNode, input) = loaded;

		// Replace non-local connectors with the one from the global metadata repository
		if (!output->isLocal()) output = *find_if(begin(outputNode->connectorMetadata()), end(outputNode->connectorMetadata()), [output](auto& c) { return c->hash() == output->hash(); });
		if (!input->isLocal()) input = *find_if(begin(inputNode->connectorMetadata()), end(inputNode->connectorMetadata()), [input](auto& c) { return c->hash() == input->hash(); });

		impl_->connection_ = make_tuple(outputNode, output, inputNode, input);
	}

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