#pragma once

#include <stdexcept>
#include <string>

#include "logging/LoggingCore.h"

#define ASSERT(x, ...) { if(x == true) { RENDERER_CRITICAL("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); } }

#define ASSERT_SUCCESS_AND_THROW(expression, message)\
		if (!(expression)) {\
			RENDERER_CRITICAL("{0} ({1}): {2}", __FILE__, __LINE__, message);\
		}\
