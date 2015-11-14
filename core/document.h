#pragma once
#include "static.h"
#include "node.h"

BEGIN_NAMESPACE(Core)

class Document
{
public:
	using connections_t = std::vector<ConnectionPtr>;

private:
	struct Impl;

public:
	Document();
	~Document();

	Document(const Document& rhs);
	Document& operator=(const Document& rhs);

	Document(Document&& rhs);
	Document& operator=(Document&& rhs);

	const NodePtr& root() const noexcept;
	const tree_t& nodes() const noexcept;
	const connections_t& connections() const noexcept;

	NodePtr parent(const Node& node) const noexcept;
	NodePtr parent(const Property& prop) const noexcept;
	NodePtr parent(const ConnectorMetadata& connectorMetadata) const noexcept;
	NodePtr child(const Node& parent, size_t index) const noexcept;
	bool exists(const Node& node) const noexcept;
	size_t childIndex(const Node& node) const noexcept;
	size_t childIndex(const Property& prop) const noexcept;
	size_t childIndex(const ConnectorMetadata& connectorMetadata) const noexcept;
	size_t childCount(const Node& node) const noexcept;
	size_t totalChildCount(const Node& node) const noexcept;

	class Builder
	{
		struct BuilderImpl;

	public:
		using mutate_fn = std::function<void(Node::Builder&)>;

		explicit Builder(const Document& d);
		~Builder();

		Builder(const Builder& rhs);
		Builder& operator=(const Builder& rhs);

		Builder(Builder&& rhs);
		Builder& operator=(Builder&& rhs);

		void mutate(NodePtr node, mutate_fn fn) const noexcept;

		void insertBefore(NodePtr before, std::initializer_list<NodePtr> nodes) noexcept;
		void append(std::initializer_list<NodePtr> nodes) noexcept;
		void append(NodePtr parent, std::initializer_list<NodePtr> nodes) noexcept;
		void moveAfter(NodePtr after, std::initializer_list<NodePtr> nodes) noexcept;

		void erase(std::initializer_list<NodePtr> nodes) noexcept;
		void eraseChildren(std::initializer_list<NodePtr> nodes) noexcept;
		void reparent(NodePtr parent, std::initializer_list<NodePtr> nodes) noexcept;

		void connect(ConnectionPtr connection);

		void fixupConnections() const;

	private:
		Builder() = default;
		friend class Document;

		std::unique_ptr<Impl> impl_;
		std::unique_ptr<BuilderImpl> builderImpl_;
	};

	Document(Builder&& rhs);
	Document& operator=(Builder&& rhs);

	static Document buildRootDocument(NodePtr root) noexcept;

#ifdef _DEBUG
	void dumpTree() const;
#endif

private:
	friend class cereal::access;
	template<class Archive> void save(Archive& archive) const;
	template<class Archive>	void load(Archive& archive);

	static tree_t::iterator iteratorFor(const tree_t& tree, const Node& node) noexcept;
	std::unique_ptr<Impl> impl_;
};

END_NAMESPACE(Core)