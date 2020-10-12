#pragma once

#include "Logger.h"

//////// RENDERER LOGGING ////////
#define RENDERER_TRACE(...)			vr::Logger::GetRendererLogger()->trace(__VA_ARGS__)
#define RENDERER_INFO(...)			vr::Logger::GetRendererLogger()->info(__VA_ARGS__)

#ifdef ENABLE_DEBUG_LOGGING
#define RENDERER_DEBUG(...)			vr::Logger::GetRendererLogger()->debug(__VA_ARGS__)
#else
#define RENDERER_DEBUG(...)
#endif

#define RENDERER_WARN(...)			vr::Logger::GetRendererLogger()->warn(__VA_ARGS__)
#define RENDERER_ERROR(...)			vr::Logger::GetRendererLogger()->error(__VA_ARGS__)
#define RENDERER_CRITICAL(...)		vr::Logger::GetRendererLogger()->critical(__VA_ARGS__)

//////// SCENE LOGGING ////////
#define APPLICATION_TRACE(...)		vr::Logger::GetSceneLogger()->trace(__VA_ARGS__)
#define APPLICATION_INFO(...)		vr::Logger::GetSceneLogger()->info(__VA_ARGS__)
#define APPLICATION_DEBUG(...)		vr::Logger::GetSceneLogger()->debug(__VA_ARGS__)
#define APPLICATION_WARN(...)		vr::Logger::GetSceneLogger()->warn(__VA_ARGS__)
#define APPLICATION_ERROR(...)		vr::Logger::GetSceneLogger()->error(__VA_ARGS__)
#define APPLICATION_CRITICAL(...)	vr::Logger::GetSceneLogger()->critical(__VA_ARGS__)