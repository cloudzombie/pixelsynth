#pragma once
#include "static.h"
#include "property.h"

BEGIN_NAMESPACE(Core)

struct Metadata;

class Node
{
	struct Impl;

public:
	using properties_t = std::map<Hash, std::shared_ptr<const Property>>;

    Node(Metadata* metadata);
	~Node();

	Node(const Node& rhs);
	Node& operator=(const Node& rhs);

	Node(Node&& rhs);
	Node& operator=(Node&& rhs);

	const properties_t& properties() const;
	const Property& prop(const Hash hash) const;

	class Builder
	{
	public:
		Builder() = delete;
		explicit Builder(const Node& d);
		~Builder();

		Builder(const Builder& rhs);
		Builder& operator=(const Builder& rhs);

		Builder(Builder&& rhs);
		Builder& operator=(Builder&& rhs);

		std::shared_ptr<Property::Builder> mutateProperty(const Hash hash) noexcept;

	private:
		friend class Node;
		std::unique_ptr<Impl> impl_;
	};

	Node(Builder&& rhs);
	Node& operator=(Builder&& rhs);

private:
    std::unique_ptr<Impl> impl_;
};

END_NAMESPACE(Core)