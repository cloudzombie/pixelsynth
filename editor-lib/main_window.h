#pragma once
#include "static.h"

class MainWindow: public QObject
{
	Q_OBJECT

public:
	MainWindow();

signals:
	void bla();
};