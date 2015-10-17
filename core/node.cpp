#include "node.h"
#include "metadata.h"
#include "factory.h"

using Core::Node;
using Core::Uuid;
using Core::Factory;
using Core::HashValue;
using Core::PropertyPtr;
using Core::Metadata;
using Core::ConnectorMetadata;
using Core::ConnectorMetadataCollection;
using Core::ConnectorMetadataPtr;
using Core::PropertyMetadata;
using Builder = Node::Builder;

struct Node::Impl
{
	Uuid uuid_;
	HashValue nodeType_;
	properties_t properties_;
	ConnectorMetadataCollection* sharedConnectorMetadata_;
	ConnectorMetadataCollection localConnectorMetadata_;

	// cache
	ConnectorMetadataCollection combinedConnectorMetadata_;
	size_t combinedHash_;
};

Node::Node()
	: impl_(std::make_unique<Impl>())
{}

Node::Node(HashValue nodeType)
	: impl_(std::make_unique<Impl>())
{
	impl_->uuid_ = uuid4();
	setNodeType(nodeType);
}

void Node::setNodeType(HashValue nodeType)
{
	impl_->nodeType_ = nodeType;

	auto metadata = Factory::metadata(nodeType);
	if (metadata)
	{
		for (auto&& meta : metadata->propertyMetadataCollection)
		{
			auto p = std::make_shared<Property>(nodeType, meta->hash());
			impl_->properties_.emplace_back(p);
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

Uuid Node::uuid() const noexcept
{
	return impl_->uuid_;
}

HashValue Node::nodeType() const noexcept
{
	return impl_->nodeType_;
}

const Node::properties_t& Node::properties() const
{
	return impl_->properties_;
}

const ConnectorMetadataCollection& Node::connectorMetadata() const
{
	auto localHash = std::hash<ConnectorMetadataCollection>()(impl_->localConnectorMetadata_);
	if (localHash != impl_->combinedHash_)
	{
		impl_->combinedConnectorMetadata_.clear();
		if (impl_->sharedConnectorMetadata_) impl_->combinedConnectorMetadata_.insert(end(impl_->combinedConnectorMetadata_), begin(*impl_->sharedConnectorMetadata_), end(*impl_->sharedConnectorMetadata_));
		impl_->combinedConnectorMetadata_.insert(end(impl_->combinedConnectorMetadata_), begin(impl_->localConnectorMetadata_), end(impl_->localConnectorMetadata_));
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

void Builder::addProperty(PropertyMetadata::Builder&& propertyMetadata) noexcept
{
	auto meta = propertyMetadata.build();
	auto p = std::make_shared<Property>(impl_->nodeType_, meta->hash(), meta);
	impl_->properties_.emplace_back(p);
}

void Builder::mutateProperty(const HashValue hash, mutate_fn fn) noexcept
{
	auto it = find_if(begin(impl_->properties_), end(impl_->properties_), property_eq_hash(impl_->nodeType_, hash));
	assert(it != end(impl_->properties_));

	auto b = Property::Builder(**it);
	fn(b);

	impl_->properties_.erase(it);
	impl_->properties_.insert(it, std::make_shared<Property>(std::move(b)));
}

void Builder::addConnector(ConnectorMetadata::Builder&& connector) noexcept
{
	impl_->localConnectorMetadata_.emplace_back(connector.withLocal(true).build());
}

///

template<class Archive>
void Node::save(Archive& archive) const
{
	archive(impl_->uuid_);
	archive(impl_->nodeType_);
	archive(impl_->properties_);
	archive(impl_->localConnectorMetadata_);
}

template<class Archive>
void Node::load(Archive& archive)
{
	archive(impl_->uuid_);

	HashValue nodeType;
	archive(nodeType);
	setNodeType(nodeType);

	std::vector<MutablePropertyPtr> props;
	archive(props);
	impl_->properties_.clear();
	for (auto&& p : props) impl_->properties_.emplace_back(p);

	std::vector<MutableConnectorMetadataPtr> localConnectors;
	archive(localConnectors);
	for (auto& c : localConnectors) impl_->localConnectorMetadata_.emplace_back(c);
}

template void Node::save<cereal::XMLOutputArchive>(cereal::XMLOutputArchive& archive) const;
template void Node::load<cereal::XMLInputArchive>(cereal::XMLInputArchive& archive);
