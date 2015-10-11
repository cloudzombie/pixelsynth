#include <editor-lib/static.h>
#include <editor-lib/main_window.h>
#include <core/metadata.h>
#include <core/factory.h>

using namespace Core;

struct DummyNode
{
	using p = PropertyMetadata::Builder;
	using c = ConnectorMetadata::Builder;

	static PropertyMetadataCollection propertyMetadata()
	{
		return
		{
			p("$Title").ofType<std::string>().build()
		};
	}

	static ConnectorMetadataCollection connectorMetadata()
	{
		return
		{
			c("Out", ConnectorType::Output).build(),
			c("In", ConnectorType::Input).build()
		};
	}

	static Metadata* metadata()
	{
		static auto m = Metadata
		{
			propertyMetadata(),
			connectorMetadata()
		};
		return &m;
	}
};

int main(int argc, char *argv[])
{
	Log::setConsoleInstance(spdlog::level::debug);
	DefineNode(DummyNode);

	QApplication app(argc, argv);
	Editor::MainWindow ui;
	return app.exec();
}