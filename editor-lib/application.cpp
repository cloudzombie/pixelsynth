#include "application.h"

using Editor::Application;

Application::Application(int argc, char *argv[])
	: QApplication(argc, argv)
{
	project_.setMutationCallback([&](auto mutationInfo)
	{
		emit projectMutated(mutationInfo);
	});
}