#include "node.h"
#include "metadata.h"

using Node = Core::Node;
using Hash = Core::Hash;
using Property = Core::Property;
using Metadata = Core::Metadata;
using Builder = Node::Builder;

struct Node::Impl
{
	properties_t properties_;
};

Node::Node(Metadata* metadata)
	: impl_(std::make_unique<Impl>())
{
	for (auto&& meta: metadata->propertyMetadataCollection)
	{
		auto p = std::make_shared<Property>(&meta);
		impl_->properties_[meta.hash()] = p;
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

const Property& Node::prop(const Hash hash) const
{
	return *impl_->properties_.find(hash)->second;
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

std::shared_ptr<Property::Builder> Builder::mutateProperty(const Hash hash) noexcept
{
	auto it = impl_->properties_.find(hash);
	assert(it != end(impl_->properties_));
	std::shared_ptr<Property::Builder> ptr(new Property::Builder(*it->second), [this, it](Property::Builder* b)
	{
		it->second = std::make_shared<Property>(std::move(*b));
		delete b;
	});
	return ptr;
}