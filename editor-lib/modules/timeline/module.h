#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules)
class Metadata;
BEGIN_NAMESPACE(Timeline)

std::unique_ptr<Metadata> registerModule();

END_NAMESPACE(Timeline)
END_NAMESPACE(Editor) END_NAMESPACE(Modules)