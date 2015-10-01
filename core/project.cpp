#include "project.h"
#include "mutation-info.h"

using Core::Document;
using Core::MutationInfo;
using Core::NodePtr;
using Core::Project;

Project::Project()
	: root_(std::make_shared<Node>(HashValue()))
{
	history_.push_back(Document::buildRootDocument(root_));
}

void Project::undo() noexcept
{
	assert(history_.size());

	redoStack_.push(history_.back());
	history_.pop_back();
	
	if (mutationCallback_) mutationCallback_(MutationInfo::compare(redoStack_.top(), current()));
}

void Project::redo() noexcept
{
	assert(redoStack_.size());

	history_.push_back(redoStack_.top());
	redoStack_.pop();

	if (mutationCallback_) mutationCallback_(MutationInfo::compare(history_.back(), current()));
}

const Document& Project::current() const noexcept
{
	assert(history_.size());
	return history_.back();
}

void Project::mutate(mutate_fn fn) noexcept
{
	auto b = Document::Builder(current());
	fn(b);
	b.fixupConnections();
	history_.push_back(std::move(b));

	if (mutationCallback_)
	{
		auto prev = history_.size() >= 2 ? history_[history_.size() - 2] : Document();
		mutationCallback_(MutationInfo::compare(prev, current()));
	}
}

void Project::setMutationCallback(mutation_callback_fn fn) noexcept
{
	mutationCallback_ = fn;
}