#pragma once
#include "static.h"

#include "node.h"
#include "connection.h"

BEGIN_NAMESPACE(Core)

class Document
{
	struct Impl;

public:
	using tree_t = tree<NodePtr>;
	using connections_t = std::vector<ConnectionPtr>;

	Document();
	~Document();

	Document(const Document& rhs);
	Document& operator=(const Document& rhs);

	Document(Document&& rhs);
	Document& operator=(Document&& rhs);

	const tree_t& nodes() const noexcept;
	const connections_t& connections() const noexcept;

	NodePtr parent(NodePtr node) const noexcept;
	size_t childCount(NodePtr node = nullptr) const noexcept;

	class Builder
	{
		struct BuilderImpl;

	public:
		explicit Builder(const Document& d);
		~Builder();

		Builder(const Builder& rhs);
		Builder& operator=(const Builder& rhs);

		Builder(Builder&& rhs);
		Builder& operator=(Builder&& rhs);

		std::shared_ptr<Node::Builder> mutate(NodePtr node) noexcept;

		void append(NodePtr parent, std::initializer_list<NodePtr> nodes) noexcept;
		void erase(std::initializer_list<NodePtr> nodes) noexcept;
		void eraseChildren(std::initializer_list<NodePtr> nodes) noexcept;
		void reparent(NodePtr parent, std::initializer_list<NodePtr> nodes) noexcept;

		void connect(ConnectionPtr connection);

		void fixConnections();

	private:
		Builder() = default;
		friend class Document;

		std::unique_ptr<Impl> impl_;
		std::unique_ptr<BuilderImpl> builderImpl_;
	};

	Document(Builder&& rhs);
	Document& operator=(Builder&& rhs);

	static Document buildRootDocument(NodePtr root) noexcept;

protected:
	static tree_t::iterator iteratorFor(const tree_t& tree, NodePtr node) noexcept;

private:
	std::unique_ptr<Impl> impl_;
};

END_NAMESPACE(Core)