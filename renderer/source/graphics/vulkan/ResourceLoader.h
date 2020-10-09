#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include <stb_image.h>

namespace vr
{
	class ResourceLoader
	{
	public:

		static std::vector<char> ReadFile(const std::string& filename);
		static stbi_uc* LoadTextureFile(const std::string filename, int* const width, int* const height, VkDeviceSize* const imageSize);
	};
}
