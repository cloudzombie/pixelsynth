#pragma once
#include "static.h"
#include "document.h"

BEGIN_NAMESPACE(Core)

class Project
{
public:
	using history_t = std::vector<Document>;

	void undo() noexcept;
	const Document& current() const noexcept;
	std::shared_ptr<Document::Builder> mutate() noexcept;

private:
	history_t history_ { 1 };
};

END_NAMESPACE(Core)