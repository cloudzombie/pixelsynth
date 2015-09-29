#pragma once
#include "static.h"
#include "document.h"

BEGIN_NAMESPACE(Core)

class Project
{
public:
	using history_t = std::vector<Document>;
	using redohistory_t = std::stack<Document>;
	using mutate_fn = std::function<void(Document::Builder&)>;

	Project();

	void undo() noexcept;
	void redo() noexcept;
	const Document& current() const noexcept;
	void mutate(mutate_fn fn) noexcept;

private:	
	friend class cereal::access;
	template<class Archive>
	void save(Archive& archive) const
	{
		archive(root_);
		archive(current());
	}

	template<class Archive>
	void load(Archive& archive)
	{
		MutableNodePtr root;
		archive(root);
		root_ = root;

		Document d;
		archive(d);
		history_ = { d };
	}

	history_t history_;
	redohistory_t redoStack_;
	NodePtr root_;
};

END_NAMESPACE(Core)