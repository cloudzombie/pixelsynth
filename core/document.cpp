#include "document.h"

using Core::Document;
using Core::Node;
using Builder = Document::Builder;

struct Document::Impl
{
	tree_t nodes_;
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

const Document::tree_t& Document::nodes() const noexcept
{
	return impl_->nodes_;
}

Document::tree_t& Builder::nodes() const noexcept
{
	return impl_->nodes_;
}

/////////////////////////////////////////////////////////
// Builder boilerplate
/////////////////////////////////////////////////////////

Builder::Builder(const Document& d)
	: impl_(std::make_unique<Impl>(*d.impl_))
{
}

Builder::~Builder() = default;

Builder::Builder(const Builder& rhs)
	: impl_(std::make_unique<Impl>(*rhs.impl_))
{}

Builder& Builder::operator=(const Builder& rhs)
{
	*impl_ = *rhs.impl_;
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

std::shared_ptr<Node::Builder> Builder::mutate(std::shared_ptr<const Node> node) noexcept
{
	std::shared_ptr<Node::Builder> ptr(new Node::Builder(*node), [this, node](Node::Builder* b)
	{
		// Construct the new node
		auto&& newNode = std::make_shared<Node>(std::move(*b));

		// Replace it in the tree
		auto pos = find(begin(impl_->nodes_), end(impl_->nodes_), node);
		impl_->nodes_.replace(pos, newNode);
		delete b;
	});
	return ptr;
}

void Builder::erase(std::shared_ptr<const Node> node) noexcept
{
	//impl_->nodes_.erase(node);
}
