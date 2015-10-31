#pragma once
#include "static.h"

struct MutationProject: Core::Project
{
	void applyMutationsTo(size_t maxMutation);
	void applyMutation(size_t mutationIndex);
	void applyMutationsFromTo(size_t start, size_t end);

	std::array<Core::NodePtr, 500> a, b, c, between_ab, between_ab2;
};