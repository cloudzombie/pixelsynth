#include "application.h"
#include <editor-lib/main_window.h>

using Core::Project;
using Editor::Application;
using Editor::MainWindow;

Application::Application(int argc, char *argv[])
	: QApplication(argc, argv)
{
	clear();
}

void Application::clear()
{
	if (mainWindow_) delete mainWindow_;

	project_ = Project();
	mainWindow_ = new MainWindow(*this);

	project_.setMutationCallback([&](auto mutationInfo)
	{
		emit projectMutated(mutationInfo);
	});

	emit projectChanged();
}

void Application::load(QString filename)
{
	QFile file(filename);
	if (!file.open(QFile::ReadOnly)) return;

	QString contents = file.readAll();

	clear();
	auto emptyDocument = project_.current();

	{
		std::stringstream s;
		std::string strContents = contents.toStdString();
		s << strContents;
		cereal::JSONInputArchive archive(s);
		archive(project_);
	}

	project_.emitMutationsComparedTo(emptyDocument);
}

void Application::save(QString filename)
{
	std::stringstream s;
	{
		cereal::JSONOutputArchive archive(s);
		archive(project_);
	}

	QFile file(filename);
	if (file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream stream(&file);
		stream << s.str().c_str();
	}
}