#include "property.h"
#include "metadata.h"
#include "factory.h"

using Core::Property;
using Core::PropertyMetadata;
using Core::Factory;
using Core::Frame;
using Core::HashValue;
using Core::PropertyValue;
using Builder = Property::Builder;

Property::Property()
	: impl_(std::make_unique<Impl>())
{}

Property::Property(HashValue nodeType, HashValue propertyType)
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

struct Interpolator
{
	explicit Interpolator(float alpha, PropertyValue& p, PropertyValue& n, PropertyValue& pp, PropertyValue& nn)
		: alpha(alpha)
		, p_(p)
		, n_(n)
		, pp_(pp)
		, nn_(nn)
	{}

	float alpha;
	PropertyValue& p_;
	PropertyValue& n_;
	PropertyValue& pp_;
	PropertyValue& nn_;

	template <typename T>
	PropertyValue operator()(const T& _)
	{
		T& p = *p_.target<T>();
		T& n = *n_.target<T>();
		T& pp = *pp_.target<T>();
		T& nn = *nn_.target<T>();

		float alpha2 = alpha * alpha;
		auto a0 = (pp * -0.5f) + (p * 1.5f) - (n * 1.5f) + (nn * 0.5f);
		auto a1 = pp - p * 2.5f + n * 2.0f - nn * 0.5f;
		auto a2 = pp * -0.5f + n * 0.5f;
		auto a3 = p;

		return static_cast<T>(a0 * alpha * alpha2 + a1 * alpha2 + a2 * alpha + a3);
	}
};

template <>
inline PropertyValue Interpolator::operator()<std::string>(const std::string& _)
{
	return p_;
}

PropertyValue Property::getPropertyValue(Frame frame) const noexcept
{
	if (!impl_->keys_.size()) return impl_->metadata_->defaultValue();

	auto exactFrame = impl_->keys_.find(frame);
	if (exactFrame != impl_->keys_.end()) return exactFrame->second;

	auto next = impl_->keys_.upper_bound(frame);

	// Beyond last item
	if (next == impl_->keys_.cend())
	{
		--next;
		return next->second;
	}

	// Before first
	if (next == impl_->keys_.cbegin())
	{
		return next->second;
	}

	auto prev = next--;
	auto prevprev = prev--;
	auto nextnext = next++;

	auto alpha = (static_cast<float>(frame) - static_cast<float>(prev->first)) / (static_cast<float>(next->first) - static_cast<float>(prev->first));

	Interpolator interpolator(alpha, prev->second, next->second, prevprev->second, nextnext->second);
	return eggs::variants::apply<PropertyValue>(interpolator, prev->second);
}

const PropertyMetadata& Property::metadata() const noexcept
{
	return *impl_->metadata_;
}

void Property::setMetadata(HashValue nodeType, HashValue propertyType) noexcept
{
	impl_->nodeType_ = nodeType;
	impl_->propertyType_ = propertyType;
	auto nodeMetadata = Factory::metadata(nodeType);
	if (nodeMetadata)
	{
		impl_->metadata_ = *find_if(begin(nodeMetadata->propertyMetadataCollection), end(nodeMetadata->propertyMetadataCollection), [&](auto& m) { return m->hash() == propertyType; });
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
