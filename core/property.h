#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)
/*
template <typename T>
static T interpolate(const T& p, const T& n, const T& pp, const T& nn, float alpha)
{
	float alpha2 = alpha * alpha;
	auto a0 = (pp * -0.5f) + (p * 1.5f) - (n * 1.5f) + (nn * 0.5f);
	auto a1 = pp - p * 2.5f + n * 2.0f - nn * 0.5f;
	auto a2 = pp * -0.5f + n * 0.5f;
	auto a3 = p;

	return static_cast<T>(a0 * alpha * alpha2 + a1 * alpha2 + a2 * alpha + a3);
}

template <>
inline std::string interpolate<std::string>(const std::string& prev, const std::string& next, const std::string& prevprev, const std::string& nextnext, float alpha)
{
	return prev;
}

*/

class PropertyMetadata;

class Property
{
	struct Impl;

public:
	Property(PropertyMetadata* metadata);
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

	const PropertyMetadata& metadata() const;

private:
	PropertyValue getPropertyValue(Frame frame) const;

	std::unique_ptr<Impl> impl_;
};

END_NAMESPACE(Core)
