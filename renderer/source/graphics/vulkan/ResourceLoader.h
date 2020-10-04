#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>

namespace vr
{
	class ResourceLoader
	{
	public:

		static std::vector<char> ReadFile(const std::string& filename);
		static VkShaderModule CreateShaderModule(const std::string& filename);
	};
}
