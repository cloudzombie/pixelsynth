#include "connection.h"
#include "metadata.h"

using Core::Connection;
using Core::HashValue;
using Core::Node;
using Core::NodePtr;
using Core::ConnectorMetadata;
using Core::ConnectorMetadataPtr;

enum ConnectionTuple
{
	OUTPUT_NODE = 0,
	OUTPUT,
	INPUT_NODE,
	INPUT
};

Connection::Connection()
	: impl_(std::make_unique<Impl>())
{}

Connection::Connection(connection_t connection)
	: impl_(std::make_unique<Impl>())
{
	assert(std::get<OUTPUT>(connection));
	assert(std::get<OUTPUT>(connection)->type() == ConnectorType::Output);
	assert(std::get<INPUT>(connection));
	assert(std::get<INPUT>(connection)->type() == ConnectorType::Input);
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

NodePtr Connection::outputNode() const { return std::get<OUTPUT_NODE>(impl_->connection_); }
ConnectorMetadataPtr Connection::output() const { return std::get<OUTPUT>(impl_->connection_); }
NodePtr Connection::inputNode() const { return std::get<INPUT_NODE>(impl_->connection_); }
ConnectorMetadataPtr Connection::input() const { return std::get<INPUT>(impl_->connection_); }
