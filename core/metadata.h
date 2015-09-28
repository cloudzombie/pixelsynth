#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)

class PropertyMetadata
{
	class Data
	{
		Hash hash_;
		std::string title_;

		friend class PropertyMetadata;
		friend class Builder;
	};
public:
	explicit PropertyMetadata(const Data&& data)
		: data_(data)
	{
	}

	Hash hash() const noexcept { return data_.hash_; }
	std::string title() const noexcept { return data_.title_; }

	class Builder
	{
	public:
		Builder(const char* title) { data_.hash_ = Core::hash(title); data_.title_ = title; }

	private:
		friend class PropertyMetadata;
		Data data_;
	};

	PropertyMetadata(Builder&& rhs)
		: data_(std::move(rhs.data_))
	{}

private:
	const Data data_;
};

using PropertyMetadataCollection = std::vector<PropertyMetadata>;

struct Metadata
{
	PropertyMetadataCollection propertyMetadataCollection;
};

END_NAMESPACE(Core)
