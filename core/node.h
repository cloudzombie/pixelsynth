#pragma once
#include "static.h"
#include "property.h"
#include "metadata.h"

BEGIN_NAMESPACE(Core)

class Node
{
public:
	using properties_t = std::vector<std::shared_ptr<const Property>>;

private:
	struct Impl;

public:
    Node(HashValue nodeType);
	~Node();

	Node(const Node& rhs);
	Node& operator=(const Node& rhs);

	Node(Node&& rhs);
	Node& operator=(Node&& rhs);

	Uuid uuid() const noexcept;
	HashValue nodeType() const noexcept;

	const properties_t& properties() const;

	const ConnectorMetadataCollection& connectorMetadata() const;

	class Builder
	{
	public:
		using mutate_fn = std::function<void(Property::Builder&)>;

		Builder() = delete;
		explicit Builder(const Node& d);
		~Builder();

		Builder(const Builder& rhs);
		Builder& operator=(const Builder& rhs);

		Builder(Builder&& rhs);
		Builder& operator=(Builder&& rhs);

		void addProperty(PropertyMetadata::Builder&& propertyMetadata) noexcept;
		void mutateProperty(const HashValue hash, mutate_fn fn) noexcept;

		void addConnector(ConnectorMetadata::Builder&& connector) noexcept;

	private:
		friend class Node;
		std::unique_ptr<Impl> impl_;
	};

	Node(Builder&& rhs);
	Node& operator=(Builder&& rhs);

	friend std::ostream& operator<<(std::ostream& out, const Node& n);
	friend std::ostream& operator<<(std::ostream& out, Node* n) { out << *n; return out; }

private:
	friend class cereal::access;
	template<class Archive> void save(Archive& archive) const;
	template<class Archive>	void load(Archive& archive);

	Node();

	void setNodeType(HashValue nodeType);

	std::unique_ptr<Impl> impl_;
};

struct node_eq_uuid
{
	explicit node_eq_uuid(NodePtr compare_to): compare_to_(compare_to) { }
	bool operator()(NodePtr c1) const { return c1->uuid() == compare_to_->uuid(); }
private:
	NodePtr compare_to_;
};

END_NAMESPACE(Core)

namespace std
{
	template<>
	struct hash<Core::NodePtr>
	{
		typedef Core::NodePtr argument_type;
		typedef size_t result_type;

		result_type operator()(argument_type const& node) const
		{
			return hash<uint64_t>()(node->uuid().ab) ^ hash<uint64_t>()(node->uuid().cd);
		}
	};
}