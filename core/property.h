#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)

class PropertyMetadata;

class Property
{
public:
	using keys_t = std::map<Frame, PropertyValue>;

private:
	struct Impl
	{
		HashValue nodeType_;
		HashValue propertyType_;
		PropertyMetadataPtr metadata_;
		keys_t keys_;
	};

public:
	Property(HashValue nodeType, HashValue propertyType);
	~Property();

	Property(const Property& rhs);
	Property& operator=(const Property& rhs);

	Property(Property&& rhs);
	Property& operator=(Property&& rhs);

	class Builder
	{
	public:
		Builder() = delete;
		explicit Builder(const Property& d);
		~Builder();

		Builder(const Builder& rhs);
		Builder& operator=(const Builder& rhs);

		Builder(Builder&& rhs);
		Builder& operator=(Builder&& rhs);

		void set(Frame frame, PropertyValue value) noexcept;

	private:
		friend class Property;
		std::unique_ptr<Impl> impl_;
	};

	Property(Builder&& rhs);
	Property& operator=(Builder&& rhs);

	template <typename T>
	T get(Frame frame) const
	{
		return *getPropertyValue(frame).target<T>();
	}

	const PropertyMetadata& metadata() const noexcept;

private:
	friend class cereal::access;
	template<class Archive>
	void save(Archive& archive) const
	{
		archive(impl_->nodeType_);
		archive(impl_->propertyType_);

		archive(impl_->keys_.size());
		for (auto&& kvp : impl_->keys_)
		{
			archive(kvp.first);
			archive(kvp.second);
		}
	}

	template<class Archive>
	void load(Archive& archive)
	{
		archive(impl_->nodeType_);
		archive(impl_->propertyType_);
		setMetadata(impl_->nodeType_, impl_->propertyType_);

		size_t size;
		archive(size);
		for (size_t t = 0; t < size;t++)
		{
			Frame frame;
			PropertyValue value = defaultValue();
			archive(frame);
			archive(value);
			impl_->keys_[frame] = value;
		}
	}

	PropertyValue getPropertyValue(Frame frame) const noexcept;
	void setMetadata(HashValue nodeType, HashValue propertyType) noexcept;
	PropertyValue defaultValue() noexcept;

	Property();
	std::unique_ptr<Impl> impl_;
};

END_NAMESPACE(Core)
