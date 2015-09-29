#pragma once
#include "static.h"
#include "property.h"

BEGIN_NAMESPACE(Core)

struct Metadata;

class Node
{
public:
	using properties_t = std::map<Hash, std::shared_ptr<const Property>>;

private:
	struct Impl
	{
		Hash nodeType_;
		properties_t properties_;
		ConnectorMetadataCollection* connectorMetadata_;
	};

public:
    Node(Hash nodeType);
	~Node();

	Node(const Node& rhs);
	Node& operator=(const Node& rhs);

	Node(Node&& rhs);
	Node& operator=(Node&& rhs);

	const properties_t& properties() const;
	const Property* prop(const Hash hash) const;

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

		void mutateProperty(const Hash hash, mutate_fn fn) noexcept;

	private:
		friend class Node;
		std::unique_ptr<Impl> impl_;
	};

	Node(Builder&& rhs);
	Node& operator=(Builder&& rhs);

private:
	friend class cereal::access;
	template<class Archive>
	void save(Archive& archive) const
	{
		archive(impl_->nodeType_);
		archive(impl_->properties_);
	}

	template<class Archive>
	void load(Archive& archive)
	{
		archive(impl_->nodeType_);
		std::map<Hash, std::shared_ptr<Property>> props;
		archive(props);
		for (auto&& kvp : props) impl_->properties_[kvp.first] = kvp.second;
	}

	Node();
	std::unique_ptr<Impl> impl_;
};

END_NAMESPACE(Core)