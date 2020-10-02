#include "Logger.h"

namespace vr {
	std::shared_ptr<spdlog::logger> Logger::sRendererLogger;
	std::shared_ptr<spdlog::logger> Logger::sSceneLogger;

	void Logger::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");

		sRendererLogger = spdlog::stdout_color_mt("RENDERER");
		sRendererLogger->set_level(spdlog::level::trace);

		sSceneLogger = spdlog::stdout_color_mt("SCENE");
		sSceneLogger->set_level(spdlog::level::trace);
	}
}