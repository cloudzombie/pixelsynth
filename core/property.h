#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)

class PropertyMetadata;

class Property
{
public:
	using keys_t = std::map<Frame, PropertyValue>;

private:
	struct Impl;

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

		void setAnimated(bool animated) noexcept;

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
	bool samePropertyHash(const PropertyPtr other) const noexcept;
	bool samePropertyHash(const HashValue otherNodeType, const HashValue otherPropertyType) const noexcept;
	HashValue nodeType() const noexcept;
	HashValue propertyType() const noexcept;

	friend std::ostream& operator<<(std::ostream& out, const Property& p);
	friend std::ostream& operator<<(std::ostream& out, Property* p) { out << *p; return out; }

private:
	friend class cereal::access;
	template<class Archive> void save(Archive& archive) const;
	template<class Archive>	void load(Archive& archive);

	friend struct property_eq_hash;

	PropertyValue getPropertyValue(Frame frame) const noexcept;
	void setMetadata(HashValue nodeType, HashValue propertyType) noexcept;
	PropertyValue defaultValue() noexcept;

	Property();
	std::unique_ptr<Impl> impl_;
};

struct property_eq_hash
{
	explicit property_eq_hash(HashValue nodeType, HashValue propertyType): nodeType_(nodeType), propertyType_(propertyType) { }
	explicit property_eq_hash(PropertyPtr other): nodeType_(other->nodeType()), propertyType_(other->propertyType()) { }
	bool operator()(PropertyPtr c1) const { return c1->samePropertyHash(nodeType_, propertyType_); }
private:
	HashValue nodeType_;
	HashValue propertyType_;
};

END_NAMESPACE(Core)