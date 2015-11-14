#pragma once
#include "static.h"

struct MutationProject: Core::Project
{
	static const size_t NUM_MUTATIONS = 32;

	void applyMutationsTo(size_t maxMutation);
	void applyMutation(size_t mutationIndex);
	void applyMutationsFromTo(size_t start, size_t end);

	std::array<Core::NodePtr, 500> a, b, c, d, between_ab, between_ab2;
};