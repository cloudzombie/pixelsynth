#include "project.h"
#include "mutation_info.h"

using Core::Document;
using Core::MutationInfo;
using Core::NodePtr;
using Core::Project;

Project::Project()
	: root_(std::make_shared<Node>(HashValue()))
{
	history_group_t group = { "New project", { Document::buildRootDocument(root_) } };
	history_.push_back(group);
}

void Project::undo() noexcept
{
	assert(history_.size() > 1);

	auto prevCurrent = current();
	redoStack_.push(history_.back());
	history_.pop_back();
	
	if (mutationCallback_) mutationCallback_(MutationInfo::compare(prevCurrent, current()));
}

void Project::redo() noexcept
{
	assert(redoStack_.size());

	auto prevCurrent = current();
	history_.push_back(redoStack_.top());
	redoStack_.pop();

	if (mutationCallback_) mutationCallback_(MutationInfo::compare(prevCurrent, current()));
}

const Document& Project::current() const noexcept
{
	assert(history_.size());
	return history_.back().second;
}

void Project::mutate(mutate_fn fn, std::string description) noexcept
{
	mutate({ fn }, description);
}

void Project::mutate(std::initializer_list<mutate_fn> fns, std::string description) noexcept
{
	auto originalState = current();

	bool needToReplace = false;
	for (auto&& fn : fns)
	{
		auto b = Document::Builder(current());
		fn(b);
		b.fixupConnections();

		if (needToReplace) history_.pop_back();
		history_.push_back({ description, std::move(b) });
		needToReplace = true;
	}

	if (mutationCallback_)
	{
		mutationCallback_(MutationInfo::compare(originalState, current()));
	}
}

void Project::setMutationCallback(mutation_callback_fn fn) noexcept
{
	mutationCallback_ = fn;
}

///

template<class Archive>
void Project::save(Archive& archive) const
{
	archive(root_);
	archive(current());
}

template<class Archive>
void Project::load(Archive& archive)
{
	MutableNodePtr root;
	archive(root);
	root_ = root;

	Document d;
	archive(d);
	history_ = { { "New project", { d } } };
}

template void Project::save<cereal::XMLOutputArchive>(cereal::XMLOutputArchive& archive) const;
template void Project::load<cereal::XMLInputArchive>(cereal::XMLInputArchive& archive);
