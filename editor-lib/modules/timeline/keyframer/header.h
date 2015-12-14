#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

BEGIN_NAMESPACE(Keyframer)

class Header: public QHeaderView
{
public:
	explicit Header(const Model& model, QWidget* parent);

private:
	void update();

	const Model& model_;
	const Core::Document* document_;
	QWidget* widget_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline) END_NAMESPACE(Keyframer)