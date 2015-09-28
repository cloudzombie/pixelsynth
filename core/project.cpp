#include "project.h"

using Core::Project;
using Core::NodePtr;
using Core::Document;

Project::Project()
	: root_(std::make_shared<Node>())
{
	history_.push_back(Document::buildRootDocument(root_));
}

void Project::undo() noexcept
{
	assert(history_.size());
	redoStack_.push(history_.back());
	history_.pop_back();
}

void Project::redo() noexcept
{
	assert(redoStack_.size());
	history_.push_back(redoStack_.top());
	redoStack_.pop();
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
		b->fixConnections();
		history_.push_back(std::move(*b));
		delete b;
	});
	return ptr;
}