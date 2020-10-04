#include "ResourceLoader.h"
#include "utility/RendererCoreUtility.h"
#include "assertions.h"

#include <fstream>

namespace vr
{
	std::vector<char> ResourceLoader::ReadFile(const std::string& filename)
	{
		// open stream from given file
		// std::ios::binary tells stream to read file as binary
		// std::ios::ate tells stream to start reading from end of file
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (!file.is_open())
		{
			THROW("FAILED TO OPEN FILE: filename");
		}

		// get current read position and use to resize file buffer
		size_t filesize = (size_t)file.tellg();
		std::vector<char> buffer(filesize);

		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();

		return buffer;
	}

	VkShaderModule CreateShaderModule(const VkDevice& logicalDevice, VkAllocationCallbacks* allocationCallbacks, const std::string& filename)
	{
		std::vector<char> code = ResourceLoader::ReadFile(filename);

		VkShaderModule shaderModule = {};
		VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = code.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const unsigned int*> (code.data());

		CHECK_RESULT(vkCreateShaderModule(logicalDevice, &shaderModuleCreateInfo, allocationCallbacks, &shaderModule), "RESOURCE CREATION FAILED: SHADER MODULE (" + filename + ")");
		RENDERER_DEBUG("RESOURCE CREATED: SHADER MODULE (" + filename + ")");
	}
}