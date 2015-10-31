#include "property.h"
#include "metadata.h"
#include "factory.h"

using Core::Property;
using Core::PropertyMetadata;
using Core::Factory;
using Core::Frame;
using Core::HashValue;
using Core::PropertyPtr;
using Core::PropertyMetadataPtr;
using Core::PropertyValue;
using Builder = Property::Builder;

struct Property::Impl
{
	HashValue nodeType_;
	HashValue propertyType_;
	PropertyMetadataPtr metadata_;
	keys_t keys_;
	bool animated_ {};
};

Property::Property()
	: impl_(std::make_unique<Impl>())
{}

Property::Property(HashValue nodeType, HashValue propertyType, PropertyMetadataPtr metadata)
	: impl_(std::make_unique<Impl>())
{
	setMetadata(nodeType, propertyType, metadata);
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

bool Property::samePropertyHash(const PropertyPtr other) const noexcept
{
	return impl_->nodeType_ == other->impl_->nodeType_ && impl_->propertyType_ == other->impl_->propertyType_;
}

bool Property::samePropertyHash(const HashValue otherNodeType, const HashValue otherPropertyType) const noexcept
{
	return impl_->nodeType_ == otherNodeType && impl_->propertyType_ == otherPropertyType;
}

HashValue Property::nodeType() const noexcept { return impl_->nodeType_; }
HashValue Property::propertyType() const noexcept { return impl_->propertyType_; }

void Property::setMetadata(HashValue nodeType, HashValue propertyType, PropertyMetadataPtr metadata) noexcept
{
	impl_->nodeType_ = nodeType;
	impl_->propertyType_ = propertyType;
	if (metadata)
	{
		impl_->metadata_ = metadata;
	}
	else
	{
		auto nodeMetadata = Factory::metadata(nodeType);
		if (nodeMetadata)
		{
			impl_->metadata_ = *find_if(begin(nodeMetadata->propertyMetadataCollection), end(nodeMetadata->propertyMetadataCollection), [&](auto& m) { return m->hash() == propertyType; });
		}
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

void Builder::setAnimated(bool animated) noexcept
{
	impl_->animated_ = animated;
}

///

namespace cereal
{
	template<class Archive>
	struct PropertyValueArchiver
	{
		explicit PropertyValueArchiver(Archive& archive)
			: archive(archive)
		{}

		template <typename T>
		void operator()(T& t)
		{
			archive(t);
		}

		Archive& archive;
	};

	template<class Archive>
	void serialize(Archive& archive, PropertyValue& p)
	{
		PropertyValueArchiver<Archive> fun(archive);
		eggs::variants::apply(fun, p);
	}

	template <class Archive>
	void serialize(Archive& ar, glm::vec2& vec2)
	{
		ar(vec2.x, vec2.y);
	}

	template <class Archive>
	void serialize(Archive& ar, glm::vec3& vec3)
	{
		ar(vec3.x, vec3.y, vec3.z);
	}
}

template<class Archive>
void Property::save(Archive& archive) const
{
	archive(impl_->nodeType_);
	archive(impl_->propertyType_);

	archive(impl_->keys_.size());
	for (auto&& kvp : impl_->keys_)
	{
		archive(kvp.first);
		archive(kvp.second);
	}

	archive(impl_->animated_);
}

template<class Archive>
void Property::load(Archive& archive)
{
	archive(impl_->nodeType_);
	archive(impl_->propertyType_);
	setMetadata(impl_->nodeType_, impl_->propertyType_);

	size_t size;
	archive(size);
	for (size_t t = 0; t < size; t++)
	{
		Frame frame;
		PropertyValue value = defaultValue();
		archive(frame);
		archive(value);
		impl_->keys_[frame] = value;
	}

	archive(impl_->animated_);
}

template void Property::save<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive) const;
template void Property::load<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive);
