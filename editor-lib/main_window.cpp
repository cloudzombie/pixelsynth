#include "main_window.h"
#include "modules/timeline/widget.h"

using Editor::MainWindow;

MainWindow::MainWindow()
{
	timeline_ = new Modules::Timeline::Widget(this);
	setCentralWidget(timeline_);

	buttonDock_ = new QDockWidget(this);
	button_ = new QPushButton(buttonDock_);
	button_->setText("mutate");
	addDockWidget(Qt::TopDockWidgetArea, buttonDock_);

	connect(button_, &QPushButton::clicked, this, [&]()
	{
		static_cast<Modules::Timeline::Widget*>(timeline_)->mutate();
	});

	showMaximized();
}