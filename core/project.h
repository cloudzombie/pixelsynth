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
	history_t history_;
	redohistory_t redoStack_;
	NodePtr root_;
};

END_NAMESPACE(Core)