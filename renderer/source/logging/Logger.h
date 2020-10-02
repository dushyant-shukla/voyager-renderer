#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

namespace vr {
	class Logger {
	public:

		Logger() {
		};

		~Logger() {
		}

		static void Init();

		static std::shared_ptr<spdlog::logger>& GetRendererLogger() {
			return sRendererLogger;
		}

		static std::shared_ptr<spdlog::logger>& GetSceneLogger() {
			return sSceneLogger;
		}

	private:

		static std::shared_ptr<spdlog::logger> sRendererLogger;
		static std::shared_ptr<spdlog::logger> sSceneLogger;
	};
}