#pragma once

#include <vulkan/vulkan.h>
#include "logging/LoggingCore.h"

#define CHECK_RESULT(expression, message)												\
		if (VK_SUCCESS != expression) {													\
			RENDERER_CRITICAL("{0} ({1}): {2}", __FILE__, __LINE__, message);			\
			throw std::runtime_error((message));										\
		}																				\

#define UINT_SIZE_CAST(size) static_cast<unsigned int> (size)