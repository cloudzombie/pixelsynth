#pragma once
#include "static.h"
#include "document.h"

BEGIN_NAMESPACE(Core)

class Project
{
public:
	using history_t = std::vector<Document>;
	using redohistory_t = std::stack<Document>;

	Project();

	void undo() noexcept;
	void redo() noexcept;
	const Document& current() const noexcept;
	std::shared_ptr<Document::Builder> mutate() noexcept;

private:
	history_t history_;
	redohistory_t redoStack_;
	NodePtr root_;
};

END_NAMESPACE(Core)