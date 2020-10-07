#include "ShaderModule.h"
#include "ResourceLoader.h"
#include "utility/RendererCoreUtility.h"

vr::ShaderModule::ShaderModule(const VkDevice& device, VkAllocationCallbacks* allocationCallbacks, std::string filename)
	: mLogicalDevice(device), mAllocationCallbacks(allocationCallbacks), mModule(), mFilename(filename)
{
	std::vector<char> code = ResourceLoader::ReadFile(filename);

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const unsigned int*> (code.data());

	CHECK_RESULT(vkCreateShaderModule(mLogicalDevice, &shaderModuleCreateInfo, mAllocationCallbacks, &mModule), "RESOURCE CREATION FAILED: SHADER MODULE (" + filename + ")");
	RENDERER_DEBUG("RESOURCE CREATED: SHADER MODULE (" + filename + ")");
}

vr::ShaderModule::~ShaderModule()
{
	//vkDestroyShaderModule(mLogicalDevice, mModule, mAllocationCallbacks);
	//RENDERER_DEBUG("RESOURCE DESTROYED: SHADER MODULE (" + mFilename + ")");
}

const VkShaderModule vr::ShaderModule::GetShaderModule()
{
	return mModule;
}

std::string& vr::ShaderModule::GetFilename()
{
	return mFilename;
}