#pragma once
#include "static.h"

struct MutationProject: Core::Project
{
	void applyMutationsTo(size_t maxMutation);
	void applyMutation(size_t mutationIndex);
	void applyMutationsFromTo(size_t start, size_t end);

	Core::NodePtr a0, b0, c0;
	Core::NodePtr a3, a4, a5, a6, b6;
	Core::NodePtr a8, b8, c8;
	Core::NodePtr a9, a10, a11, between_ab12, a13;
};