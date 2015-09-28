#include "project.h"

using Core::Project;
using Core::Document;

void Project::undo() noexcept
{
	assert(history_.size());
	history_.pop_back();
}

const Document& Project::current() const noexcept
{
	assert(history_.size());
	return history_.back();
}

std::shared_ptr<Document::Builder> Project::mutate() noexcept
{
	std::shared_ptr<Document::Builder> ptr(new Document::Builder(current()), [this](Document::Builder* b)
	{
		history_.push_back(std::move(*b));
		delete b;
	});
	return ptr;
}