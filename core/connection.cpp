#include "connection.h"
#include "metadata.h"
#include "versioninfo.h"

using Core::Connection;
using Core::HashValue;
using Core::Node;
using Core::NodePtr;
using Core::ConnectorMetadata;
using Core::ConnectorMetadataPtr;

enum ConnectionTuple
{
	OUTPUT_NODE_ = 0,
	OUTPUT_,
	INPUT_NODE_,
	INPUT_
};

Connection::Connection()
	: impl_(std::make_unique<Impl>())
{}

Connection::Connection(connection_t connection)
	: impl_(std::make_unique<Impl>())
{
	assert(std::get<OUTPUT_>(connection));
	assert(std::get<OUTPUT_>(connection)->type() == ConnectorType::Output);
	assert(std::get<INPUT_>(connection));
	assert(std::get<INPUT_>(connection)->type() == ConnectorType::Input);
	impl_->connection_ = connection;
}

Connection::~Connection() = default;

Connection::Connection(const Connection& rhs)
	: impl_(std::make_unique<Impl>(*rhs.impl_))
{}

Connection& Connection::operator=(const Connection& rhs)
{
	*impl_ = *rhs.impl_;
	return *this;
}

Connection::Connection(Connection&& rhs) = default;
Connection& Connection::operator=(Connection&& rhs) = default;

const Connection::connection_t& Connection::connection() const noexcept
{
	return impl_->connection_;
}

NodePtr Connection::outputNode() const { return std::get<OUTPUT_NODE_>(impl_->connection_); }
ConnectorMetadataPtr Connection::output() const { return std::get<OUTPUT_>(impl_->connection_); }
NodePtr Connection::inputNode() const { return std::get<INPUT_NODE_>(impl_->connection_); }
ConnectorMetadataPtr Connection::input() const { return std::get<INPUT_>(impl_->connection_); }
