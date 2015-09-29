#include "static.h"

#include <core/factory.h>

using namespace Core;
#include "testnode.h"

int main(int argc, char* argv[])
{
	DefineNode(TestNode);

	// Run the tests.
	return bandit::run(argc, argv);
}