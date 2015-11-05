#include "document.h"
#include "connection.h"

using Core::Document;
using Core::Node;
using Core::NodePtr;
using Core::Property;
using Core::PropertyPtr;
using Core::Connection;
using Core::ConnectionPtr;
using Core::HashValue;
using Core::ConnectorMetadata;
using Builder = Document::Builder;

struct Document::Impl
{
	tree_t nodes_;
	connections_t connections_;
};

Document::Document()
	: impl_(std::make_unique<Impl>())
{
}

Document::~Document() = default;

Document::Document(const Document& rhs)
	: impl_(std::make_unique<Impl>(*rhs.impl_))
{}

Document& Document::operator=(const Document& rhs)
{
	*impl_ = *rhs.impl_;
	return *this;
}

Document::Document(Document&& rhs) = default;
Document& Document::operator=(Document&& rhs) = default;

const NodePtr& Document::root() const noexcept
{
	return *begin(impl_->nodes_);
}

const Core::tree_t& Document::nodes() const noexcept
{
	return impl_->nodes_;
}

const Document::connections_t& Document::connections() const noexcept
{
	return impl_->connections_;
}

NodePtr Document::parent(const Node& node) const noexcept
{
	auto it = iteratorFor(this->nodes(), node);
	auto parent = tree_t::parent(it);
	if (parent.node == nullptr) return nullptr; // root has no parent
	return *parent;
}

NodePtr Document::parent(const Property& prop) const noexcept
{
	for (auto&& node: this->nodes())
	{
		auto it = find_if(cbegin(node->properties()), cend(node->properties()), [&](auto& p) { return p.get() == &prop; });
		if (it != cend(node->properties())) return node;
	}
	return nullptr;
}

NodePtr Document::parent(const ConnectorMetadata& connectorMetadata) const noexcept
{
	for (auto&& node : this->nodes())
	{
		auto it = find_if(cbegin(node->connectorMetadata()), cend(node->connectorMetadata()), [&](auto& p) { return p.get() == &connectorMetadata; });
		if (it != cend(node->connectorMetadata())) return node;
	}
	return nullptr;
}

NodePtr Document::child(const Node& parent, size_t index) const noexcept
{
	assert(childCount(parent) > index);
	auto it = iteratorFor(this->nodes(), parent);
	return *tree_t::child(it, index);
}

bool Document::exists(const Node& node) const noexcept
{
	return std::find_if(cbegin(this->nodes()), cend(this->nodes()), [&](auto& n) { return n.get() == &node; }) != cend(this->nodes());
}

size_t Document::childIndex(const Node& node) const noexcept
{
	auto it = iteratorFor(this->nodes(), node);
	return this->nodes().index(it);
}

size_t Document::childIndex(const Property& prop) const noexcept
{
	auto node = parent(prop);
	size_t index = 0;
	for (auto&& p: node->properties())
	{
		if (p.get() == &prop) return index;
		index++;
	}
	return -1;
}

size_t Document::childIndex(const ConnectorMetadata& connectorMetadata) const noexcept
{
	auto node = parent(connectorMetadata);
	size_t index = 0;
	for (auto&& p : node->connectorMetadata())
	{
		if (p.get() == &connectorMetadata) return index;
		index++;
	}
	return -1;
}

size_t Document::childCount(const Node& node) const noexcept
{
	return this->nodes().number_of_children(iteratorFor(this->nodes(), node));
}

size_t Document::totalChildCount(const Node& node) const noexcept
{
	return this->nodes().size(iteratorFor(this->nodes(), node)) - 1; // - 1 because it includes the node itself
}

Document Document::buildRootDocument(NodePtr root) noexcept
{
	Document d;
	d.impl_->nodes_.set_head(root);
	return d;
}

Core::tree_t::iterator Document::iteratorFor(const Core::tree_t& tree, const Node& node) noexcept
{
	auto it = std::find_if(cbegin(tree), cend(tree), [&node](auto& n) { return n.get() == &node; });
	assert(it != end(tree));
	return it;
}

/////////////////////////////////////////////////////////
// Builder boilerplate
/////////////////////////////////////////////////////////

struct Builder::BuilderImpl
{
	std::map<NodePtr, NodePtr> mutatedNodes_;
};

Builder::Builder(const Document& d)
	: impl_(std::make_unique<Impl>(*d.impl_))
	, builderImpl_(std::make_unique<BuilderImpl>())
{
}

Builder::~Builder() = default;

Builder::Builder(const Builder& rhs)
	: impl_(std::make_unique<Impl>(*rhs.impl_))
	, builderImpl_(std::make_unique<BuilderImpl>(*rhs.builderImpl_))
{}

Builder& Builder::operator=(const Builder& rhs)
{
	*impl_ = *rhs.impl_;
	*builderImpl_ = *rhs.builderImpl_;
	return *this;
}

Builder::Builder(Builder&& rhs) = default;
Builder& Builder::operator=(Builder&& rhs) = default;

Document::Document(Builder&& rhs)
	: impl_(move(rhs.impl_))
{}

Document& Document::operator=(Builder&& rhs)
{
	impl_ = move(rhs.impl_);
	return *this;
}

void Builder::mutate(NodePtr node, mutate_fn fn) const noexcept
{
	auto b = Node::Builder(*node);
	fn(b);

	// Construct the new node
	auto&& newNode = std::make_shared<Node>(std::move(b));
	builderImpl_->mutatedNodes_[node] = newNode;

	// Replace it in the tree
	auto pos = find(begin(impl_->nodes_), end(impl_->nodes_), node);
	assert(pos != end(impl_->nodes_));
	impl_->nodes_.replace(pos, newNode);
}

void Builder::fixupConnections() const
{
	connections_t fixed;

	// Replace it in the connections
	for (auto& conPtr: impl_->connections_)
	{
		NodePtr outputNode;
		ConnectorMetadataPtr output;
		NodePtr inputNode;
		ConnectorMetadataPtr input;
		tie(outputNode, output, inputNode, input) = conPtr->connection();

		// Has the output or input node mutated?
		auto hasMutated = builderImpl_->mutatedNodes_.find(outputNode);
		if (hasMutated != end(builderImpl_->mutatedNodes_)) outputNode = hasMutated->second;

		hasMutated = builderImpl_->mutatedNodes_.find(inputNode);
		if (hasMutated != end(builderImpl_->mutatedNodes_)) inputNode = hasMutated->second;

		// Has the output or input node been deleted?
		if (find(begin(impl_->nodes_), end(impl_->nodes_), outputNode) == end(impl_->nodes_)) continue;
		if (find(begin(impl_->nodes_), end(impl_->nodes_), inputNode) == end(impl_->nodes_)) continue;

		auto con = make_tuple(outputNode, output, inputNode, input);
		if (con != conPtr->connection())
		{
			fixed.emplace_back(std::make_shared<const Connection>(con));
		}
		else
		{
			fixed.emplace_back(conPtr);
		}
	}

	impl_->connections_ = fixed;
}

void Builder::insertBefore(NodePtr before, std::initializer_list<NodePtr> nodes) noexcept
{
	assert(before);
	auto beforePos = iteratorFor(impl_->nodes_, *before);

	for (auto&& node : nodes)
	{
		impl_->nodes_.insert(beforePos, node);
	}
}

void Builder::append(std::initializer_list<NodePtr> nodes) noexcept
{
	append(*impl_->nodes_.begin(), nodes);
}

void Builder::append(NodePtr parent, std::initializer_list<NodePtr> nodes) noexcept
{
	assert(parent);
	auto parentPos = iteratorFor(impl_->nodes_, *parent);

	for (auto&& node : nodes)
	{
		impl_->nodes_.append_child(parentPos, node);
	}
}

void Builder::erase(std::initializer_list<NodePtr> nodes) noexcept
{
	for (auto&& node : nodes)
	{
		auto pos = find(begin(impl_->nodes_), end(impl_->nodes_), node);
		assert(pos != end(impl_->nodes_));
		impl_->nodes_.erase(pos);
	}
}

void Builder::eraseChildren(std::initializer_list<NodePtr> nodes) noexcept
{
	for (auto&& node : nodes) impl_->nodes_.erase_children(find(begin(impl_->nodes_), end(impl_->nodes_), node));
}

void Builder::reparent(NodePtr parent, std::initializer_list<NodePtr> nodes) noexcept
{
	auto parentPos = iteratorFor(impl_->nodes_, *parent);

	for (auto&& node: nodes)
	{
		auto it = iteratorFor(impl_->nodes_, *node);
		impl_->nodes_.reparent(parentPos, it, impl_->nodes_.next_sibling(it));
	}
}

void Builder::connect(ConnectionPtr connection)
{
	impl_->connections_.emplace_back(connection);
}

///

template<class Archive>
void Document::save(Archive& archive) const
{
	auto it = impl_->nodes_.begin();

	// root
	archive(*it++);

	std::vector<std::pair<NodePtr, NodePtr>> nodes;
	std::for_each(it, end(impl_->nodes_), [&](auto& node) { nodes.emplace_back(std::make_pair(this->parent(*node), node)); });
	archive(nodes);
	archive(impl_->connections_);
}

template<class Archive>
void Document::load(Archive& archive)
{
	MutableNodePtr root;
	archive(root);
	impl_->nodes_.set_head(root);

	std::vector<std::pair<MutableNodePtr, MutableNodePtr>> nodes;
	archive(nodes);

	for (auto&& kvp : nodes)
	{
		auto&& parent = kvp.first;
		auto&& child = kvp.second;
		impl_->nodes_.insert(end(impl_->nodes_), child);

		auto parentPos = iteratorFor(impl_->nodes_, *parent);
		auto it = iteratorFor(impl_->nodes_, *child);
		impl_->nodes_.reparent(parentPos, it, impl_->nodes_.next_sibling(it));
	}

	std::vector<MutableConnectionPtr> connections;
	archive(connections);
	for (auto&& con : connections) impl_->connections_.emplace_back(con);
}

template void Document::save<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive) const;
template void Document::load<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive);

