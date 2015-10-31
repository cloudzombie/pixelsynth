#pragma once
#include "static.h"
#include <core/project.h>

BEGIN_NAMESPACE(Editor)

class Application: public QApplication
{
	Q_OBJECT

public:
	explicit Application(int argc, char *argv[]);

	Core::Project& project() noexcept { return project_; }

signals:
	void projectMutated(std::shared_ptr<Core::MutationInfo> mutationInfo);

private:
	Core::Project project_;
};

END_NAMESPACE(Editor)