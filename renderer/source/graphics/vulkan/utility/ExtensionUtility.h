#pragma once

#include "RendererCoreUtility.h"
#include <array>
#include <vector>

namespace vr
{
	class ExtensionUtility
	{
	public:

		static std::vector<const char*> VALIDATION_LAYERS;
		static bool CheckValidationLayerSupport();
		static std::vector<const char*> GetRequiredExtensions();
		static bool CheckInstanceExtentionSupport(std::vector<const char*>* checkExtensions);
	};
}
