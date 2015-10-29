#pragma once
#include "static.h"
#include <core/project.h>

BEGIN_NAMESPACE(Editor)

class Application: public QApplication
{
public:
	explicit Application(int argc, char *argv[]);

	Core::Project& activeProject() noexcept { return activeProject_; }

private:
	Core::Project activeProject_;
};

END_NAMESPACE(Editor)