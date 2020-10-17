#include "ShaderModule.h"
#include "ResourceLoader.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"

vr::ShaderModule::ShaderModule(std::string filename)
	:mFilename(filename)
{
	std::vector<char> code = ResourceLoader::ReadFile(SHADER_BASE_PATH + filename);

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const unsigned int*> (code.data());

	CHECK_RESULT(vkCreateShaderModule(LOGICAL_DEVICE, &shaderModuleCreateInfo, ALLOCATION_CALLBACK, &mModule), "RESOURCE CREATION FAILED: SHADER MODULE (" + filename + ")");
	RENDERER_DEBUG("RESOURCE CREATED: SHADER MODULE (" + filename + ")");
}

vr::ShaderModule::~ShaderModule()
{
	// clean up is done in pipeline
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