#include "TextureSampler.h"
#include "utility/RendererCoreUtility.h"

namespace vr
{
	TextureSampler::TextureSampler()
	{
	}

	TextureSampler::~TextureSampler()
	{
		if (mSampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(mLogicalDevice, mSampler, mAllocationCallbacks);
			RENDERER_DEBUG("RESOURCE DESCTROYED: TEXTURE SAMPLER");
		}
	}

	void TextureSampler::CreateDefault(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks)
	{
		mLogicalDevice = logicalDevice;
		mAllocationCallbacks = allocationCallbacks;

		mSamplerInfo = {};
		mSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		mSamplerInfo.magFilter = VK_FILTER_LINEAR;									// how to render when image is magnified on screen
		mSamplerInfo.minFilter = VK_FILTER_LINEAR;									// how to render when image is minified on screen
		mSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;					// how to handle texture wrap in U(x) direction
		mSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;					// how to handle texture wrap in V(y) direction
		mSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;					// how to handle texture wrap in W(z) direction
		mSamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;				// border beyond texture, used when address mode is set to CLAMP TO BORDER
		mSamplerInfo.unnormalizedCoordinates = VK_FALSE;							// normalized coordinates = TRUE
		mSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;					// mip map interpolation mode
		mSamplerInfo.mipLodBias = 0.0f;												// level of detail bias for mip level
		mSamplerInfo.minLod = 0.0f;													// minimum level of detail to pick mip level
		mSamplerInfo.maxLod = 0.0f;													// maximum level of detail to pick mip level
		mSamplerInfo.anisotropyEnable = VK_TRUE;									// enable anisotropy
		mSamplerInfo.maxAnisotropy = 16;											// anisotropy sample level
		mSamplerInfo.compareEnable = VK_FALSE;										// If a comparison function is enabled, then texels will first be compared to a value, and the result of that comparison is used in filtering operations.
		mSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		CHECK_RESULT(vkCreateSampler(mLogicalDevice, &mSamplerInfo, nullptr, &mSampler), "RESOURCE CREATION FAILED: TEXTURE SAMPLER");
		RENDERER_DEBUG("RESOURCE CREATED: TEXTURE SAMPLER");
	}

	TextureSampler& TextureSampler::Initialize(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks)
	{
		return *this;
	}

	const VkSampler& TextureSampler::GetVulkanSampler()
	{
		return mSampler;
	}
}