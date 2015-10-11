#pragma once
#include "static.h"

#include <spdlog/spdlog.h>

class Log
{
public:
	static void setConsoleInstance(spdlog::level::level_enum level);
	static std::shared_ptr<spdlog::logger> instance;
};

#define LOG Log::instance