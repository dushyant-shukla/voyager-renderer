#include "Pipeline.h"

namespace vr
{
	Pipeline::Pipeline(const VkDevice& device, VkAllocationCallbacks* allocationCallbacks)
		: mLogicalDevice(device), mAllocationCallbacks(allocationCallbacks)
	{}

	Pipeline::~Pipeline() {}

	Pipeline& Pipeline::AddShaderStage(const VkShaderStageFlagBits& shaderStage, std::string filename)
	{
		ShaderModule vertexShader(mLogicalDevice, nullptr, filename);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = shaderStage;
		vertShaderStageInfo.module = vertexShader.GetShaderModule();
		vertShaderStageInfo.pName = "main";

		mShaderStages.push_back(vertShaderStageInfo);
		return *this;
	}

	Pipeline& Pipeline::AddVertexInputBindingDescription(const unsigned int binding, const unsigned int stride, const VkVertexInputRate& inputRate)
	{
		VkVertexInputBindingDescription description = {};
		description.binding = binding;
		description.stride = stride;
		description.inputRate = inputRate;
		mBindingDescriptions.push_back(description);

		return *this;
	}

	Pipeline& Pipeline::AddVertexInputAttributeDescription(const unsigned int binding, const VkFormat& format, const unsigned int location, const unsigned int offset)
	{
		VkVertexInputAttributeDescription description = {};
		description.binding = binding;
		description.format = format;
		description.location = location;
		description.offset = offset;
		mAttributeDescriptions.push_back(description);
		return *this;
	}

	Pipeline& Pipeline::ConfigureInputAssembly(const VkPrimitiveTopology& topology, const VkBool32& primitiveRestartEnable, VkPipelineInputAssemblyStateCreateFlags flags, void* next)
	{
		mInputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mInputAssemblyCreateInfo.topology = topology;
		mInputAssemblyCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
		mInputAssemblyCreateInfo.flags = flags;
		mInputAssemblyCreateInfo.pNext = next;

		return *this;
	}

	Pipeline& Pipeline::ConfigureViewport(const VkExtent2D& swapchainExtent)
	{
		// A viewport basically describes the region of the framebuffer that the output will be rendered to.
		// This will almost always be (0, 0) to (width, height) and in this tutorial that will also be the case.

		// The size of the swap chain and its images may differ from the WIDTH and HEIGHT of the window.
		// The swap chain images will be used as framebuffers later on, so we should stick to their size.

		VkViewport viewport = {};
		viewport.x = 0.0f;										// x origin
		viewport.y = 0.0f;										// y origin
		viewport.width = (float)swapchainExtent.width;					// viewport width
		viewport.height = (float)swapchainExtent.height;				// viewport height
		viewport.minDepth = 0.0f;								// min framebuffer depth
		viewport.maxDepth = 1.0f;								// max framebuffer depth

		// Scissor
		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };								// offset to use region from
		scissor.extent = swapchainExtent;								// Extent to describe region to use, starting at offset

		mViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mViewportStateCreateInfo.viewportCount = 1;
		mViewportStateCreateInfo.pViewports = &viewport;
		mViewportStateCreateInfo.scissorCount = 1;
		mViewportStateCreateInfo.pScissors = &scissor;

		return *this;
	}

	Pipeline& Pipeline::ConfigureRasterizer(const VkBool32& depthClampEnable, const VkBool32& rasterizerDiscardEnable,
		const VkPolygonMode& polygonMode, VkCullModeFlags cullMode, const VkFrontFace& frontface,
		const VkBool32& depthBiasEnable, const float depthBiasConstantFactor, const float depthBiasClamp,
		const float depthBiasSlopeFactor, const float lineWidth,
		VkPipelineRasterizationStateCreateFlags flags, void* next)
	{
		mRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mRasterizationStateCreateInfo.depthClampEnable = depthClampEnable;
		mRasterizationStateCreateInfo.rasterizerDiscardEnable = rasterizerDiscardEnable;
		mRasterizationStateCreateInfo.polygonMode = polygonMode;
		mRasterizationStateCreateInfo.cullMode = cullMode;
		mRasterizationStateCreateInfo.frontFace = frontface;
		mRasterizationStateCreateInfo.depthBiasEnable = depthBiasEnable;
		mRasterizationStateCreateInfo.depthBiasConstantFactor = depthBiasConstantFactor;
		mRasterizationStateCreateInfo.depthBiasClamp = depthBiasClamp;
		mRasterizationStateCreateInfo.depthBiasSlopeFactor = depthBiasSlopeFactor;
		mRasterizationStateCreateInfo.lineWidth = lineWidth;
		mRasterizationStateCreateInfo.pNext = next;

		return *this;
	}

	Pipeline& Pipeline::ConfigureMultiSampling(VkSampleCountFlagBits rasterizationSamples, const VkBool32& sampleShadingEnable,
		const float minSampleShading, VkSampleMask* pSampleMask,
		const VkBool32& alphaToCoverageEnable, const VkBool32& alphaToOneEnable, void* next)
	{
		mMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mMultisampling.sampleShadingEnable = sampleShadingEnable;
		mMultisampling.rasterizationSamples = rasterizationSamples;
		mMultisampling.minSampleShading = minSampleShading; // Optional
		mMultisampling.pSampleMask = pSampleMask; // Optional
		mMultisampling.alphaToCoverageEnable = alphaToCoverageEnable; // Optional
		mMultisampling.alphaToOneEnable = alphaToOneEnable; // Optional
		mMultisampling.pNext = next;
		mMultisampling.flags = flags;

		return *this;
	}

	Pipeline& Pipeline::AddColorBlendAttachmentState(const VkBool32& blendEnable, const VkBlendFactor& srcColorBlendFactor,
		const VkBlendFactor& dstColorBlendFactor, const VkBlendOp& colorBlendOp,
		const VkBlendFactor& srcAlphaBlendFactor, const VkBlendFactor& dstAlphaBlendFactor,
		const VkBlendOp& alphaBlendOp, VkColorComponentFlags colorWriteMask)
	{
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = colorWriteMask;
		colorBlendAttachment.blendEnable = blendEnable;
		colorBlendAttachment.srcColorBlendFactor = srcColorBlendFactor; // Optional
		colorBlendAttachment.dstColorBlendFactor = dstColorBlendFactor; // Optional
		colorBlendAttachment.colorBlendOp = colorBlendOp; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = srcAlphaBlendFactor; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = dstAlphaBlendFactor; // Optional
		colorBlendAttachment.alphaBlendOp = alphaBlendOp; // Optional

		mColorBlendAttachments.push_back(colorBlendAttachment);

		return *this;
	}

	Pipeline& Pipeline::ConfigureColorBlend(VkStructureType sType, void* pNext, VkPipelineColorBlendStateCreateFlags flags,
		VkBool32 logicOpEnable, VkLogicOp logicOp, uint32_t attachmentCount,
		VkPipelineColorBlendAttachmentState* pAttachments,
		float blendConstant0, float blendConstant1, float blendConstant2, float blendConstant3)
	{
		mColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mColorBlending.logicOpEnable = VK_FALSE;
		mColorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		mColorBlending.attachmentCount = static_cast<unsigned int>(mColorBlendAttachments.size());
		mColorBlending.pAttachments = mColorBlendAttachments.empty() ? nullptr : mColorBlendAttachments.data();
		mColorBlending.blendConstants[0] = blendConstant0; // Optional
		mColorBlending.blendConstants[1] = blendConstant1; // Optional
		mColorBlending.blendConstants[2] = blendConstant2; // Optional
		mColorBlending.blendConstants[3] = blendConstant3; // Optional
		mColorBlending.flags = flags;
		mColorBlending.pNext = pNext;

		return *this;
	}

	void Pipeline::Create()
	{
		// input state
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<unsigned int>(mBindingDescriptions.size());
		vertexInputCreateInfo.pVertexBindingDescriptions = mBindingDescriptions.empty() ? nullptr : mBindingDescriptions.data();
		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<unsigned int> (mAttributeDescriptions.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = mAttributeDescriptions.empty() ? nullptr : mAttributeDescriptions.data();

		// dynamic state
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;
		dynamicState.pNext = nullptr;
		dynamicState.flags = 0;
	}
}