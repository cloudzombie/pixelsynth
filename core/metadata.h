#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)

class PropertyMetadata
{
	class Data
	{
		HashValue hash_;
		std::string title_;
		PropertyValue defaultValue_;

		friend bool operator==(const Data& lhs, const Data& rhs)
		{
			return lhs.hash_ == rhs.hash_
				&& lhs.title_ == rhs.title_
				&& lhs.defaultValue_ == rhs.defaultValue_;
		}

		friend bool operator!=(const Data& lhs, const Data& rhs)
		{
			return !(lhs == rhs);
		}

		friend class PropertyMetadata;
		friend class Builder;
	};

public:
	explicit PropertyMetadata(const Data&& data)
		: data_(data)
	{
	}

	HashValue hash() const noexcept { return data_.hash_; }
	std::string title() const noexcept { return data_.title_; }
	PropertyValue defaultValue() const noexcept { return data_.defaultValue_; }

	class Builder
	{
	public:
		explicit Builder(const char* title) { data_.hash_ = Core::hash(title); data_.title_ = title; }
		PropertyMetadataPtr build() noexcept { return std::make_shared<PropertyMetadata>(std::move(*this)); }

		template <typename T>
		Builder&& ofType() { data_.defaultValue_ = T(); return std::move(*this); }

	private:
		friend class PropertyMetadata;
		Data data_;
	};

	PropertyMetadata(Builder&& rhs)
		: data_(std::move(rhs.data_))
	{}

	friend std::ostream& operator<<(std::ostream& out, const PropertyMetadata& n);
	friend std::ostream& operator<<(std::ostream& out, PropertyMetadata* n) { out << *n; return out; }

	friend bool operator==(const PropertyMetadata& lhs, const PropertyMetadata& rhs)
	{
		return lhs.data_ == rhs.data_;
	}

	friend bool operator!=(const PropertyMetadata& lhs, const PropertyMetadata& rhs)
	{
		return !(lhs == rhs);
	}

private:
	const Data data_;
};

class ConnectorMetadata
{
	class Data
	{
		HashValue hash_;
		std::string title_;
		ConnectorType type_;
		bool isLocal_ { false };

		friend class ConnectorMetadata;
		friend class Builder;
	};
public:
	explicit ConnectorMetadata(const Data&& data)
		: data_(data)
	{
	}

	HashValue hash() const noexcept { return data_.hash_; }
	std::string title() const noexcept { return data_.title_; }
	ConnectorType type() const noexcept { return data_.type_; }
	bool isLocal() const noexcept { return data_.isLocal_; }

	class Builder
	{
	public:
		Builder(const char* title, ConnectorType type) { data_.hash_ = Core::hash(title); data_.title_ = title; data_.type_ = type; }
		Builder& withLocal(bool local) { data_.isLocal_ = local; return *this; }
		ConnectorMetadataPtr build() noexcept { return std::make_shared<ConnectorMetadata>(std::move(*this)); }

	private:
		friend class ConnectorMetadata;
		Data data_;
	};

	ConnectorMetadata(Builder&& rhs)
		: data_(std::move(rhs.data_))
	{}

	friend std::ostream& operator<<(std::ostream& out, const ConnectorMetadata& n);
	friend std::ostream& operator<<(std::ostream& out, ConnectorMetadata* n) { out << *n; return out; }

private:
	friend class cereal::access;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(data_.hash_);
		archive(data_.isLocal_);
		if (!data_.isLocal_) return;
		archive(data_.title_);
		archive(data_.type_);
	}

	ConnectorMetadata() = default;
	Data data_;
};

struct Metadata
{
	PropertyMetadataCollection propertyMetadataCollection;
	ConnectorMetadataCollection connectorMetadataCollection;
};

END_NAMESPACE(Core)

namespace std
{
	template<> struct hash<Core::ConnectorMetadata>
	{
		size_t operator()(const Core::ConnectorMetadata& s) const
		{
			auto h1(hash<string>()(s.title()));
			auto h2(hash<size_t>()(s.type() == Core::ConnectorType::Output ? 0 : 1));
			return h1 ^ (h2 << 1);
		}
	};

	template<> struct hash<Core::ConnectorMetadataCollection>
	{
		size_t operator()(const Core::ConnectorMetadataCollection& s) const
		{
			size_t h = 54059;
			for (auto&& item : s) h ^= hash<Core::ConnectorMetadata>()(*item);
			return h;
		}
	};

	template<> struct hash<Core::PropertyMetadata>
	{
		size_t operator()(const Core::PropertyMetadata& s) const
		{
			auto h1(hash<string>()(s.title()));
			return h1;
		}
	};

}

struct connector_metadata_eq_hash
{
	explicit connector_metadata_eq_hash(Core::ConnectorMetadataPtr ptr): ptr_(ptr) { }
	bool operator()(Core::ConnectorMetadataPtr c1) const { return c1->hash() == ptr_->hash(); }

private:
	Core::ConnectorMetadataPtr ptr_;
};