#pragma once
#include "static.h"
#include <core/project.h>

BEGIN_NAMESPACE(Editor)

class MainWindow;

class Application: public QApplication
{
	Q_OBJECT

public:
	explicit Application(int argc, char *argv[]);

	Core::Project& project() noexcept { return project_; }

	void clear();
	void load(QString filename);
	void save(QString filename);

signals:
	void projectChanged();
	void projectMutated(std::shared_ptr<Core::MutationInfo> mutationInfo);

private:
	Core::Project project_;
	MainWindow* mainWindow_ {};
};

END_NAMESPACE(Editor)