#pragma once

#include "ShaderModule.h"
#include "PipelineLayout.h"

#include <vector>

namespace vr
{
	class Pipeline
	{
	public:

		Pipeline();
		~Pipeline();

		Pipeline& AddShaderStage(const VkShaderStageFlagBits& shaderStage, std::string filename);

		Pipeline& AddVertexInputBindingDescription(const unsigned int binding, const unsigned int stride, const VkVertexInputRate& inputRate);
		Pipeline& AddVertexInputBindingDescription(const VkVertexInputBindingDescription& bindingDescription);

		Pipeline& AddVertexInputAttributeDescription(const unsigned int binding, const VkFormat& format, const unsigned int location, const unsigned int offset);
		Pipeline& AddVertexInputAttributeDescription(std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);

		Pipeline& ConfigureInputAssembly(const VkPrimitiveTopology& topology, const VkBool32& primitiveRestartEnable, VkPipelineInputAssemblyStateCreateFlags flags, void* next);

		Pipeline& ConfigureViewport(const VkExtent2D& swapchainExtent);
		Pipeline& ConfigureViewport(unsigned int viewportCount, unsigned int scissorCount, const VkExtent2D swapchainExtent, VkPipelineViewportStateCreateFlags flags = 0);

		Pipeline& ConfigureRasterizer(const VkBool32& depthClampEnable, const VkBool32& rasterizerDiscardEnable,
			const VkPolygonMode& polygonMode, VkCullModeFlags cullMode, const VkFrontFace& frontface,
			const VkBool32& depthBiasEnable, const float depthBiasConstantFactor, const float depthBiasClamp,
			const float depthBiasSlopeFactor, const float lineWidth,
			VkPipelineRasterizationStateCreateFlags flags, void* next);

		Pipeline& ConfigureMultiSampling(VkSampleCountFlagBits rasterizationSamples, const VkBool32& sampleShadingEnable,
			const float minSampleShading, VkSampleMask* pSampleMask, VkPipelineMultisampleStateCreateFlags flags,
			const VkBool32& alphaToCoverageEnable, const VkBool32& alphaToOneEnable, void* next);

		Pipeline& AddColorBlendAttachmentState(const VkBool32& blendEnable, const VkBlendFactor& srcColorBlendFactor,
			const VkBlendFactor& dstColorBlendFactor, const VkBlendOp& colorBlendOp,
			const VkBlendFactor& srcAlphaBlendFactor, const VkBlendFactor& dstAlphaBlendFactor,
			const VkBlendOp& alphaBlendOp, VkColorComponentFlags colorWriteMask);

		Pipeline& ConfigureColorBlendState(void* pNext, VkPipelineColorBlendStateCreateFlags flags,
			VkBool32 logicOpEnable, VkLogicOp logicOp,
			float blendConstant0, float blendConstant1, float blendConstant2, float blendConstant3);

		Pipeline& ConfigureDefaultDepthTesting();
		Pipeline& ConfigureDepthTesting(const VkBool32 depthEnable, const VkBool32 depthWriteEnable, const VkCompareOp compareOp);

		// TODO: TESELLATION STATE

		// should do stuff like VkPipelineVertexInputStateCreateInfo from binding and attr descriptions
		void Configure(const VkPipelineLayout& pipelineLayout, const VkRenderPass& renderPass, unsigned int subpass, const VkPipelineCreateFlags flags);

		VkPipeline& GetVulkanPipeline();

	private:

		// SHADER STAGES
		std::vector<VkPipelineShaderStageCreateInfo> mShaderStages;
		std::vector<ShaderModule> mShaderModules;

		// INPUT STATE
		std::vector<VkVertexInputBindingDescription> mBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> mAttributeDescriptions;

		// INPUT ASSEMBLY
		VkPipelineInputAssemblyStateCreateInfo mInputAssemblyCreateInfo;

		// VIEWPORT AND SCISSOR
		VkViewport mViewport;
		VkRect2D mScissor;
		VkPipelineViewportStateCreateInfo mViewportStateCreateInfo;

		// RASTERIZER
		VkPipelineRasterizationStateCreateInfo mRasterizationStateCreateInfo;

		// MULTISAMPLING
		VkPipelineMultisampleStateCreateInfo mMultisampling;

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};

		// COLOR BLENDING
		std::vector<VkPipelineColorBlendAttachmentState> mColorBlendAttachments;
		VkPipelineColorBlendStateCreateInfo mColorBlending;

		VkPipeline mPipeline = VK_NULL_HANDLE;
	};
}
