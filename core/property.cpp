#include "property.h"
#include "metadata.h"
#include "factory.h"

using Core::Property;
using Core::PropertyMetadata;
using Core::Factory;
using Core::Frame;
using Core::Hash;
using Core::PropertyValue;
using Builder = Property::Builder;

Property::Property()
	: impl_(std::make_unique<Impl>())
{}

Property::Property(Hash nodeType, Hash propertyType)
	: impl_(std::make_unique<Impl>())
{
	setMetadata(nodeType, propertyType);
}

Property::~Property() = default;

Property::Property(const Property& rhs)
	: impl_(std::make_unique<Impl>(*rhs.impl_))
{}

Property& Property::operator=(const Property& rhs)
{
	*impl_ = *rhs.impl_;
	return *this;
}

Property::Property(Property&& rhs) = default;
Property& Property::operator=(Property&& rhs) = default;

PropertyValue Property::getPropertyValue(Frame frame) const noexcept
{
	return impl_->keys_[frame];
}

const PropertyMetadata& Property::metadata() const noexcept
{
	return *impl_->metadata_;
}

void Property::setMetadata(Hash nodeType, Hash propertyType) noexcept
{
	impl_->nodeType_ = nodeType;
	impl_->propertyType_ = propertyType;
	auto nodeMetadata = Factory::metadata(nodeType);
	if (nodeMetadata)
	{
		impl_->metadata_ = &*find_if(begin(nodeMetadata->propertyMetadataCollection), end(nodeMetadata->propertyMetadataCollection), [&](auto& m) { return m.hash() == propertyType; });
	}
}

PropertyValue Property::defaultValue() noexcept
{
	return impl_->metadata_->defaultValue();
}

/////////////////////////////////////////////////////////
// Builder boilerplate
/////////////////////////////////////////////////////////

Builder::Builder(const Property& d)
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

Property::Property(Builder&& rhs)
	: impl_(move(rhs.impl_))
{}

Property& Property::operator=(Builder&& rhs)
{
	impl_ = move(rhs.impl_);
	return *this;
}

void Builder::set(Frame frame, PropertyValue value) noexcept
{
	impl_->keys_[frame] = value;
}
