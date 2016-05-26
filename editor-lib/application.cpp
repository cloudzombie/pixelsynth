#include "application.h"
#include "actions.h"
#include "modules/graph/module.h"
#include "modules/inspector/module.h"
#include "modules/timeline/module.h"

using Core::Project;
using Editor::Actions;
using Editor::Application;
using Editor::Modules::ActionFlags;
using Editor::Modules::Metadata;

Application::Application(int argc, char *argv[])
	: QApplication(argc, argv)
	, mainWindow_(nullptr)
	, prevFocus_(nullptr)
{
	QFile f(":qdarkstyle/style.qss");
	f.open(QFile::ReadOnly | QFile::Text);
	QTextStream ts(&f);
	setStyleSheet(ts.readAll());

	installEventFilter(this);
	registerModules();
	setup();
}

void Application::registerModules()
{
	moduleMetadata_.emplace_back(Modules::Graph::registerModule());
	moduleMetadata_.emplace_back(Modules::Inspector::registerModule());
	moduleMetadata_.emplace_back(Modules::Timeline::registerModule());
}

void Application::applyModules()
{
	for (auto&& metadata: moduleMetadata_)
	{
		auto widget = metadata.createWidget()(this, mainWindow_, project_);
		if (metadata.dockWidgetArea() == Qt::NoDockWidgetArea) mainWindow_->setCentralWidget(widget);
		else mainWindow_->addDockWidget(metadata.dockWidgetArea(), qobject_cast<QDockWidget*>(widget));

		auto&& src = metadata.actions();
		Modules::Metadata::action_list_t reversed_actions(src.size());
		reverse_copy(begin(src), end(src), begin(reversed_actions));
		for (auto&& actionItem : reversed_actions)
		{
			if (actionRequiresFocus(actionItem))
			{
				focusActions_[widget].emplace_back(actionItem);
			}
			else
			{
				addAction(widget, actionItem);
			}
		}
	}
}

void Application::addAction(QWidget* widget, Metadata::action_item_t actionItem)
{
	auto actionFn = std::get<0>(actionItem);
	auto actionToAdd = actionFn(this, widget);
	QString existingActionText = std::get<1>(actionItem);

	for (auto&& menuChild: mainWindow_->menuBar()->children())
	{
		auto menu = qobject_cast<QMenu*>(menuChild);
		if (!menu) continue;

		for (auto t = 0; t < menu->actions().size();t++)
		{
			auto action = menu->actions()[t];
			if (action->text() == existingActionText)
			{
				if (t + 1 < menu->actions().size())
				{
					auto after = menu->actions()[t + 1];
					menu->insertAction(after, actionToAdd);
				}
				else
				{
					menu->addAction(actionToAdd);
				}

				if (actionRequiresFocus(actionItem))
				{
					createdFocusActions_[widget].emplace_back(actionToAdd);
				}

				return;
			};
		}
	}

	// TODO: Fix this
	//throw new std::logic_error("Could not add menu item");
}

void Application::removeFocusActions(QWidget* widget)
{
	for (auto&& action : createdFocusActions_[widget])
	{
		widget->removeAction(action);
		action->deleteLater();
	}
	createdFocusActions_.erase(widget);
}

void Application::connectActions()
{
	connect(globalActions_->newFile, &QAction::triggered, this, &Application::setup);
	connect(globalActions_->openFile, &QAction::triggered, this, &Application::openFile);
	connect(globalActions_->saveFileAs, &QAction::triggered, this, &Application::saveFileAs);
	connect(globalActions_->exit, &QAction::triggered, this, &Application::quit);

	connect(globalActions_->undo, &QAction::triggered, this, [&]() { project().undo(); });
	connect(globalActions_->redo, &QAction::triggered, this, [&]() { project().redo(); });

	connect(this, &Application::projectMutated, this, [&](auto mutationInfo)
	{
		auto undoState = project().undoState();
		globalActions_->undo->setText(tr("&Undo") + " " + undoState.undoDescription.c_str());
		globalActions_->redo->setText(tr("&Redo") + " " + undoState.redoDescription.c_str());
		globalActions_->undo->setEnabled(undoState.canUndo);
		globalActions_->redo->setEnabled(undoState.canRedo);
	});
}

void Application::fillMenu() const
{
	auto fileMenu = mainWindow_->menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(globalActions_->newFile);
	fileMenu->addAction(globalActions_->openFile);
	fileMenu->addAction(globalActions_->saveFileAs);
	fileMenu->addSeparator();
	fileMenu->addAction(globalActions_->exit);

	auto editMenu = mainWindow_->menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(globalActions_->undo);
	editMenu->addAction(globalActions_->redo);
}

void Application::setup()
{
	if (mainWindow_) mainWindow_->deleteLater();
	
	// Reset focus (actions)
	while (createdFocusActions_.size()) removeFocusActions(begin(createdFocusActions_)->first);
	prevFocus_ = nullptr;

	project_ = Project();
	mainWindow_ = new QMainWindow();
	mainWindow_->menuBar()->setNativeMenuBar(false);

	globalActions_ = std::make_shared<Actions>(this);
	fillMenu();
	connectActions();
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

	setup();
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

bool Application::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::FocusIn)
	{
		QWidget* focus = qobject_cast<QWidget*>(object);

		while (focus)
		{
			auto it = focusActions_.find(focus);
			if (it != end(focusActions_))
			{
				break;
			}

			focus = focus->parentWidget();
		}

		// Did focus change to another module?
		if (focusActions_.find(focus) != end(focusActions_))
		{
			// Yes, so remove old actions and add new ones
			if (focus != prevFocus_)
			{
				if (prevFocus_) removeFocusActions(prevFocus_);
				prevFocus_ = focus;

				if (focus)
				{
					auto it = focusActions_.find(focus);
					for (auto&& action : it->second)
					{
						addAction(focus, action);
					}
				}
			}
		}
	}
	return false;
}