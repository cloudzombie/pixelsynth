#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Timeline)

class Model;

class KeyframeHeader: public QHeaderView
{
public:
	explicit KeyframeHeader(const Model& model, QWidget* parent);

private:
	void update();

	const Model& model_;
	const Core::Document* document_;
	QWidget* widget_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Timeline)