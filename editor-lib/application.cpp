#include "application.h"
#include "actions.h"

#include "modules/timeline/module.h"

using Core::Project;
using Editor::Actions;
using Editor::Application;

Application::Application(int argc, char *argv[])
	: QApplication(argc, argv)
	, actions_(std::make_shared<Actions>(this))
{
	registerModules();
	connectActions();

	clear();
}

void Application::registerModules()
{
	moduleMetadata_.insert(move(Modules::Timeline::registerModule()));
}

void Application::applyModules()
{
	for (auto&& metadata: moduleMetadata_)
	{
		auto widget = metadata->createWidget()(mainWindow_);
		if (metadata->dockWidgetArea() == Qt::NoDockWidgetArea) mainWindow_->setCentralWidget(widget);
		else mainWindow_->addDockWidget(metadata->dockWidgetArea(), qobject_cast<QDockWidget*>(widget));
	}
}

void Application::connectActions()
{
	connect(actions_->newFile, &QAction::triggered, this, &Application::clear);
	connect(actions_->openFile, &QAction::triggered, this, &Application::openFile);
	connect(actions_->saveFileAs, &QAction::triggered, this, &Application::saveFileAs);
	connect(actions_->exit, &QAction::triggered, this, &Application::quit);

	connect(actions_->undo, &QAction::triggered, this, [&]() { project().undo(); });
	connect(actions_->redo, &QAction::triggered, this, [&]() { project().redo(); });

	connect(this, &Application::projectMutated, this, [&](auto mutationInfo)
	{
		auto undoState = project().undoState();
		actions_->undo->setText(tr("&Undo") + " " + undoState.undoDescription.c_str());
		actions_->redo->setText(tr("&Redo") + " " + undoState.redoDescription.c_str());
		actions_->undo->setEnabled(undoState.canUndo);
		actions_->redo->setEnabled(undoState.canRedo);
	});
}

void Application::fillMenu(QMenuBar* menu) const
{
	auto fileMenu = menu->addMenu(tr("&File"));
	fileMenu->addAction(actions_->newFile);
	fileMenu->addAction(actions_->openFile);
	fileMenu->addAction(actions_->saveFileAs);
	fileMenu->addSeparator();
	fileMenu->addAction(actions_->exit);

	auto editMenu = menu->addMenu(tr("&Edit"));
	editMenu->addAction(actions_->undo);
	editMenu->addAction(actions_->redo);
}

void Application::clear()
{
	if (mainWindow_) mainWindow_->deleteLater();

	project_ = Project();
	mainWindow_ = new QMainWindow();
	fillMenu(mainWindow_->menuBar());
	applyModules();

	mainWindow_->showMaximized();

	project_.setMutationCallback([&](auto mutationInfo)
	{
		emit projectMutated(mutationInfo);
	});

	emit projectChanged();
}

void Application::openFile()
{
	auto filename = QFileDialog::getOpenFileName(nullptr, tr("Open project"), QString(), tr("Project files (*.json)"));
	if (!filename.isEmpty())
	{
		load(filename);
	}
}

void Application::saveFileAs()
{
	auto filename = QFileDialog::getSaveFileName(nullptr, tr("Save project"), QString(), tr("Project files (*.json)"));
	if (!filename.isEmpty())
	{
		save(filename);
	}
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