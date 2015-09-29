#include "property.h"
#include "metadata.h"

using Core::Property;
using Core::PropertyMetadata;
using Core::Frame;
using Core::PropertyValue;
using Builder = Property::Builder;

struct Property::Impl
{
	PropertyMetadata* metadata_;
	keys_t keys_;
};

Property::Property(PropertyMetadata* metadata)
	: impl_(std::make_unique<Impl>())
{
	impl_->metadata_ = metadata;
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

PropertyValue Property::getPropertyValue(Frame frame) const
{
	return impl_->keys_[frame];
}

const PropertyMetadata& Property::metadata() const
{
	return *impl_->metadata_;
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
