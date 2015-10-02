#pragma once
#include "static.h"

#include "node.h"
#include "connection.h"

BEGIN_NAMESPACE(Core)

class Document
{
public:
	using tree_t = tree<NodePtr>;
	using connections_t = std::vector<ConnectionPtr>;

private:
	struct Impl
	{
		tree_t nodes_;
		connections_t connections_;
	};

public:
	Document();
	~Document();

	Document(const Document& rhs);
	Document& operator=(const Document& rhs);

	Document(Document&& rhs);
	Document& operator=(Document&& rhs);

	const tree_t& nodes() const noexcept;
	const connections_t& connections() const noexcept;

	NodePtr parent(NodePtr node) const noexcept;
	size_t childIndex(NodePtr node) const noexcept;
	size_t childCount(NodePtr node) const noexcept;

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

		void mutate(NodePtr node, mutate_fn fn) noexcept;

		void append(std::initializer_list<NodePtr> nodes) noexcept;
		void append(NodePtr parent, std::initializer_list<NodePtr> nodes) noexcept;

		void erase(std::initializer_list<NodePtr> nodes) noexcept;
		void eraseChildren(std::initializer_list<NodePtr> nodes) noexcept;
		void reparent(NodePtr parent, std::initializer_list<NodePtr> nodes) noexcept;

		void connect(ConnectionPtr connection);

		void fixupConnections();

	private:
		Builder() = default;
		friend class Document;

		std::unique_ptr<Impl> impl_;
		std::unique_ptr<BuilderImpl> builderImpl_;
	};

	Document(Builder&& rhs);
	Document& operator=(Builder&& rhs);

	static Document buildRootDocument(NodePtr root) noexcept;

private:
	friend class cereal::access;

	template<class Archive>
	void save(Archive& archive) const
	{
		auto it = impl_->nodes_.begin();

		// root
		archive(*it++);

		std::map<NodePtr, std::vector<NodePtr>> nodes;
		std::for_each(it, end(impl_->nodes_), [&](auto& node) { nodes[this->parent(node)].emplace_back(node); });
		archive(nodes);
		archive(impl_->connections_);
	}

	template<class Archive>
	void load(Archive& archive)
	{
		MutableNodePtr root;
		archive(root);
		impl_->nodes_.set_head(root);

		std::map<MutableNodePtr, std::vector<MutableNodePtr>> nodes;
		archive(nodes);

		for (auto&& kvp: nodes)
		{
			auto&& parent = kvp.first;
			for (auto&& child : kvp.second)
			{
				auto parentPos = iteratorFor(impl_->nodes_, parent);
				impl_->nodes_.append_child(parentPos, child);
			}
		}

		std::vector<MutableConnectionPtr> connections;
		archive(connections);
		for (auto&& con : connections) impl_->connections_.emplace_back(con);

	}

	static tree_t::iterator iteratorFor(const tree_t& tree, NodePtr node) noexcept;
	std::unique_ptr<Impl> impl_;
};

END_NAMESPACE(Core)