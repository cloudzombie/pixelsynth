#pragma once
#include "static.h"
#include <core/project.h>

#include "modules/metadata.h"

BEGIN_NAMESPACE(Editor)

struct Actions;

class Application: public QApplication
{
	Q_OBJECT

public:
	explicit Application(int argc, char *argv[]);

	Core::Project& project() noexcept { return project_; }

signals:
	void projectChanged();
	void projectMutated(std::shared_ptr<Core::MutationInfo> mutationInfo);

private:
	void registerModules();
	void applyModules();

	void addActionAfter(QString existingActionText, QAction* actionToAdd) const;

	void connectActions();
	void fillMenu() const;

	void setup();
	void load(QString filename);
	void save(QString filename);

	void openFile();
	void saveFileAs();

	Core::Project project_;
	QMainWindow* mainWindow_;
	std::shared_ptr<Actions> actions_;

	std::deque<Modules::Metadata> moduleMetadata_;
};

END_NAMESPACE(Editor)