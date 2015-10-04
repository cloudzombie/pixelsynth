#pragma once
#include "static.h"

struct MutationProject: Core::Project
{
	void applyMutationsTo(size_t maxMutation);

	Core::NodePtr a0, b0, c0;
	Core::NodePtr a3, a4, a5, a6, b6;
	Core::NodePtr a8, b8, c8;
};