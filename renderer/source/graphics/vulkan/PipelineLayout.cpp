#include "PipelineLayout.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"

namespace vr
{
	PipelineLayout::PipelineLayout()
	{
	}

	PipelineLayout::~PipelineLayout()
	{
		if (mLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(LOGICAL_DEVICE, mLayout, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE DESTROYED: GRAPHICS PIPELINE LAYOUT");
		}
	}

	PipelineLayout& PipelineLayout::AddDescriptorSetLayout(const VkDescriptorSetLayout& layout)
	{
		mDescriptorSetLayouts.push_back(layout);
		return *this;
	}

	// TODO: Test this
	PipelineLayout& PipelineLayout::AddPushConstant(const VkShaderStageFlags flags, unsigned int offset, unsigned int size)
	{
		mPushConstant = VkPushConstantRange();
		mPushConstant->stageFlags = flags;
		mPushConstant->offset = offset;
		mPushConstant->size = size;
		return *this;
	}

	void PipelineLayout::Configure()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<unsigned int> (mDescriptorSetLayouts.size());
		pipelineLayoutCreateInfo.pSetLayouts = mDescriptorSetLayouts.empty() ? nullptr : mDescriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = mPushConstant.has_value() ? 1 : 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = mPushConstant.has_value() ? &mPushConstant.value() : nullptr;

		CHECK_RESULT(vkCreatePipelineLayout(LOGICAL_DEVICE, &pipelineLayoutCreateInfo, ALLOCATION_CALLBACK, &mLayout), "RESOURCE CREATION FAILED: PIPELINE LAYOUT");
		RENDERER_DEBUG("RESOURCE CREATED: PIPELINE LAYOUT");
	}

	const VkPipelineLayout& PipelineLayout::GetVulkanPipelineLayout()
	{
		return mLayout;
	}
}