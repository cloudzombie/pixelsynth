#include "document.h"
#include "connection.h"

using Core::Document;
using Core::Node;
using Core::NodePtr;
using Core::Connection;
using Core::ConnectionPtr;
using Core::HashValue;
using Core::ConnectorMetadata;
using Builder = Document::Builder;

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

const Document::tree_t& Document::nodes() const noexcept
{
	return impl_->nodes_;
}

const Document::connections_t& Document::connections() const noexcept
{
	return impl_->connections_;
}

NodePtr Document::parent(NodePtr node) const noexcept
{
	assert(node);
	auto it = iteratorFor(this->nodes(), node);
	auto parent = tree_t::parent(it);
	if (parent.node == nullptr) return nullptr; // root has no parent
	return *parent;
}

size_t Document::childCount(NodePtr node) const noexcept
{
	assert(node);
	return this->nodes().size(iteratorFor(this->nodes(), node)) - 1; // - 1 because it includes the node itself
}

Document Document::buildRootDocument(NodePtr root) noexcept
{
	Document d;
	d.impl_->nodes_.set_head(root);
	return d;
}

Document::tree_t::iterator Document::iteratorFor(const tree_t& tree, NodePtr node) noexcept
{
	assert(node);
	auto it = find(begin(tree), end(tree), node);
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

void Builder::mutate(NodePtr node, mutate_fn fn) noexcept
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

void Builder::fixupConnections()
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

void Builder::append(std::initializer_list<NodePtr> nodes) noexcept
{
	append(*impl_->nodes_.begin(), nodes);
}

void Builder::append(NodePtr parent, std::initializer_list<NodePtr> nodes) noexcept
{
	assert(parent);
	auto parentPos = iteratorFor(impl_->nodes_, parent);

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
	auto parentPos = iteratorFor(impl_->nodes_, parent);

	for (auto&& node: nodes)
	{
		impl_->nodes_.erase(iteratorFor(impl_->nodes_, node));
		impl_->nodes_.append_child(parentPos, node);
	}
}

void Builder::connect(ConnectionPtr connection)
{
	impl_->connections_.emplace_back(connection);
}