#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(Graph)

class Widget: public QWidget
{
	Q_OBJECT

public:
	Widget(QWidget* parent, Core::Project& project);

public slots:
	void projectMutated(std::shared_ptr<Core::MutationInfo> mutationInfo) const;

private:
	Core::Project& project_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(Graph)