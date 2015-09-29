#include "node.h"
#include "metadata.h"
#include "factory.h"

using Core::Node;
using Core::Factory;
using Core::Hash;
using Core::Property;
using Core::Metadata;
using Core::ConnectorMetadata;
using Core::ConnectorMetadataCollection;
using Core::ConnectorMetadataPtr;
using Builder = Node::Builder;

Node::Node()
	: impl_(std::make_unique<Impl>())
{}

Node::Node(Hash nodeType)
	: impl_(std::make_unique<Impl>())
{
	setNodeType(nodeType);
}

void Node::setNodeType(Hash nodeType)
{
	impl_->nodeType_ = nodeType;

	auto metadata = Factory::metadata(nodeType);
	if (metadata)
	{
		for (auto&& meta : metadata->propertyMetadataCollection)
		{
			auto p = std::make_shared<Property>(nodeType, meta->hash());
			impl_->properties_[meta->hash()] = p;
		}

		impl_->sharedConnectorMetadata_ = &metadata->connectorMetadataCollection;
	}
}

Node::~Node() = default;

Node::Node(const Node& rhs)
	: impl_(std::make_unique<Impl>(*rhs.impl_))
{}

Node& Node::operator=(const Node& rhs)
{
	*impl_ = *rhs.impl_;
	return *this;
}

Node::Node(Node&& rhs) = default;
Node& Node::operator=(Node&& rhs) = default;

const Node::properties_t& Node::properties() const
{
	return impl_->properties_;
}

const Property* Node::prop(const Hash hash) const
{
	auto it = impl_->properties_.find(hash);
	if (it == end(impl_->properties_)) return nullptr;
	return it->second.get();
}

const ConnectorMetadataCollection& Node::connectorMetadata() const
{
	auto localHash = std::hash<ConnectorMetadataCollection>()(impl_->localConnectorMetadata_);
	if (localHash != impl_->combinedHash_)
	{
		impl_->combinedConnectorMetadata_ = impl_->localConnectorMetadata_;
		impl_->combinedConnectorMetadata_.insert(end(impl_->combinedConnectorMetadata_), begin(*impl_->sharedConnectorMetadata_), end(*impl_->sharedConnectorMetadata_));
		impl_->combinedHash_ = localHash;
	}
	return impl_->combinedConnectorMetadata_;
}

/////////////////////////////////////////////////////////
// Builder boilerplate
/////////////////////////////////////////////////////////

Builder::Builder(const Node& d)
	: impl_(std::make_unique<Impl>(*d.impl_))
{
}

Builder::~Builder() = default;

Builder::Builder(const Builder& rhs)
	: impl_(std::make_unique<Impl>(*rhs.impl_))
{}

Builder& Builder::operator=(const Builder& rhs)
{
	*impl_ = *rhs.impl_;
	return *this;
}

Builder::Builder(Builder&& rhs) = default;
Builder& Builder::operator=(Builder&& rhs) = default;

Node::Node(Builder&& rhs)
	: impl_(move(rhs.impl_))
{}

Node& Node::operator=(Builder&& rhs)
{
	impl_ = move(rhs.impl_);
	return *this;
}

void Builder::mutateProperty(const Hash hash, mutate_fn fn) noexcept
{
	auto it = impl_->properties_.find(hash);
	assert(it != end(impl_->properties_));

	auto b = Property::Builder(*it->second);
	fn(b);
	it->second = std::make_shared<Property>(std::move(b));
}

void Builder::addConnector(ConnectorMetadata::Builder&& connector) noexcept
{
	impl_->localConnectorMetadata_.emplace_back(connector.withLocal(true).build());
}