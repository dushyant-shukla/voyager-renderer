#pragma once

#include "Logger.h"

//////// RENDERER LOGGING ////////
#define RENDERER_TRACE(...)			Logger::GetRendererLogger()->trace(__VA_ARGS__)
#define RENDERER_INFO(...)			Logger::GetRendererLogger()->info(__VA_ARGS__)

#ifdef ENABLE_DEBUG_LOGGING
#define RENDERER_DEBUG(...)			Logger::GetRendererLogger()->debug(__VA_ARGS__)
#else
#define RENDERER_DEBUG(...)
#endif

#define RENDERER_WARN(...)			Logger::GetRendererLogger()->warn(__VA_ARGS__)
#define RENDERER_ERROR(...)			Logger::GetRendererLogger()->error(__VA_ARGS__)
#define RENDERER_CRITICAL(...)		Logger::GetRendererLogger()->critical(__VA_ARGS__)

//////// SCENE LOGGING ////////
#define APPLICATION_TRACE(...)		Logger::GetSceneLogger()->trace(__VA_ARGS__)
#define APPLICATION_INFO(...)		Logger::GetSceneLogger()->info(__VA_ARGS__)
#define APPLICATION_DEBUG(...)		Logger::GetSceneLogger()->debug(__VA_ARGS__)
#define APPLICATION_WARN(...)		Logger::GetSceneLogger()->warn(__VA_ARGS__)
#define APPLICATION_ERROR(...)		Logger::GetSceneLogger()->error(__VA_ARGS__)
#define APPLICATION_CRITICAL(...)	Logger::GetSceneLogger()->critical(__VA_ARGS__)