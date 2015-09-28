#include "connection.h"
#include "metadata.h"

using Core::Connection;
using Core::Hash;
using Core::Node;
using Core::ConnectorMetadata;

enum ConnectionTuple
{
	OUTPUT_NODE = 0,
	OUTPUT,
	INPUT_NODE,
	INPUT
};

struct Connection::Impl
{
	connection_t connection_;
};

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