#include "static.h"

#include <core/factory.h>

using namespace Core;
#include "testnode.h"

int main(int argc, char* argv[])
{
	Log::setConsoleInstance(spdlog::level::info);
	//spdlog::set_level(spdlog::level::info);
	DefineNode(TestNode);

	// Run the tests.
	return bandit::run(argc, argv);
}