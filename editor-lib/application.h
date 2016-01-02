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

	void addAction(QWidget* widget, Modules::Metadata::action_item_t actionItem);
	void removeFocusActions(QWidget* widget);

	void connectActions();
	void fillMenu() const;

	void setup();
	void load(QString filename);
	void save(QString filename);

	void openFile();
	void saveFileAs();

	bool eventFilter(QObject* object, QEvent* event);

	Core::Project project_;
	QMainWindow* mainWindow_;
	std::shared_ptr<Actions> globalActions_;

	std::deque<Modules::Metadata> moduleMetadata_;
	std::map<QWidget*, std::vector<QAction*>> createdFocusActions_;
	std::map<QWidget*, Modules::Metadata::action_list_t> focusActions_;
	QWidget* prevFocus_;
};

END_NAMESPACE(Editor)