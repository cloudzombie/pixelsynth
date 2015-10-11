#include "log.h"
using namespace Core;

std::shared_ptr<spdlog::logger> Log::instance;

void Log::setConsoleInstance(spdlog::level::level_enum level)
{
	spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] (%t) %v");

	instance = spdlog::create("logger", {
		std::make_shared<spdlog::sinks::stdout_sink_mt>()
	});

	spdlog::set_level(level);
}