#pragma once
#include "static.h"

#include "node.h"

BEGIN_NAMESPACE(Core)

class Document
{
	struct Impl;

public:
	using tree_t = tree<std::shared_ptr<const Node>>;

	Document();
	~Document();

	Document(const Document& rhs);
	Document& operator=(const Document& rhs);

	Document(Document&& rhs);
	Document& operator=(Document&& rhs);

	const tree_t& nodes() const noexcept;

	class Builder
	{
	public:
		Builder() = delete;
		explicit Builder(const Document& d);
		~Builder();

		Builder(const Builder& rhs);
		Builder& operator=(const Builder& rhs);

		Builder(Builder&& rhs);
		Builder& operator=(Builder&& rhs);

		tree_t& nodes() const noexcept;
		std::shared_ptr<Node::Builder> mutate(std::shared_ptr<const Node> node) noexcept;
		void erase(std::shared_ptr<const Node> node) noexcept;

	private:
		friend class Document;
		std::unique_ptr<Impl> impl_;
	};

	Document(Builder&& rhs);
	Document& operator=(Builder&& rhs);

private:
	std::unique_ptr<Impl> impl_;
};

END_NAMESPACE(Core)