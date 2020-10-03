#pragma once

#include "RendererCoreUtility.h"
#include <array>
#include <vector>

namespace vr
{
	//// VALIDATION LAYER UTILITY FUNCTION ////
	const std::array<const char*, 1> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
	bool CheckValidationLayerSupport();

	std::vector<const char*> GetRequiredExtensions();
}
