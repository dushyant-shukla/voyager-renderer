#include "TextureSampler.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"

namespace vr
{
	TextureSampler::TextureSampler()
	{
		mSamplerInfo = {};
		mSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	}

	TextureSampler::~TextureSampler()
	{
		if (mSampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(LOGICAL_DEVICE, mSampler, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE DESCTROYED: TEXTURE SAMPLER");
		}
	}

	void TextureSampler::CreateDefault()
	{
		mSamplerInfo = {};
		mSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		mSamplerInfo.magFilter = VK_FILTER_LINEAR;									// how to render when image is magnified on screen
		mSamplerInfo.minFilter = VK_FILTER_LINEAR;									// how to render when image is minified on screen
		mSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;					// how to handle texture wrap in U(x) direction
		mSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;					// how to handle texture wrap in V(y) direction
		mSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;					// how to handle texture wrap in W(z) direction
		mSamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;				// border beyond texture, used when address mode is set to CLAMP TO BORDER
		mSamplerInfo.unnormalizedCoordinates = VK_FALSE;							// normalized coordinates = TRUE
		mSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;					// mip map interpolation mode
		mSamplerInfo.mipLodBias = 0.0f;												// level of detail bias for mip level
		mSamplerInfo.minLod = 0.0f;													// minimum level of detail to pick mip level
		mSamplerInfo.maxLod = 0.0f;													// maximum level of detail to pick mip level
		mSamplerInfo.anisotropyEnable = VK_TRUE;									// enable anisotropy
		mSamplerInfo.maxAnisotropy = 16;											// anisotropy sample level
		mSamplerInfo.compareEnable = VK_FALSE;										// If a comparison function is enabled, then texels will first be compared to a value, and the result of that comparison is used in filtering operations.
		mSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		CHECK_RESULT(vkCreateSampler(LOGICAL_DEVICE, &mSamplerInfo, ALLOCATION_CALLBACK, &mSampler), "RESOURCE CREATION FAILED: TEXTURE SAMPLER");
		RENDERER_DEBUG("RESOURCE CREATED: TEXTURE SAMPLER");
	}

	TextureSampler& TextureSampler::AddSamplerCreateFlags(const VkSamplerCreateFlags flags)
	{
		mSamplerInfo.flags = flags;
		return *this;
	}

	TextureSampler& TextureSampler::AddMagFilter(const VkFilter filter)
	{
		mSamplerInfo.magFilter = filter;
		return *this;
	}

	TextureSampler& TextureSampler::AddMinFilter(const VkFilter filter)
	{
		mSamplerInfo.minFilter = filter;
		return *this;
	}

	TextureSampler& TextureSampler::AddSamplerMipmapMode(const VkSamplerMipmapMode mode)
	{
		mSamplerInfo.mipmapMode = mode;
		return *this;
	}

	TextureSampler& TextureSampler::AddSamplerAddressModeU(const VkSamplerAddressMode mode)
	{
		mSamplerInfo.addressModeU = mode;
		return *this;
	}

	TextureSampler& TextureSampler::AddSamplerAddressModeV(const VkSamplerAddressMode mode)
	{
		mSamplerInfo.addressModeV = mode;
		return *this;
	}

	TextureSampler& TextureSampler::AddSamplerAddressModeW(const VkSamplerAddressMode mode)
	{
		mSamplerInfo.addressModeW = mode;
		return *this;
	}

	TextureSampler& TextureSampler::AddMipLodBias(const float bias)
	{
		mSamplerInfo.mipLodBias = bias;
		return *this;
	}

	TextureSampler& TextureSampler::EnableAnistropy(const VkBool32 enable)
	{
		mSamplerInfo.anisotropyEnable = enable;
		return *this;
	}

	TextureSampler& TextureSampler::AddMaxAnistropy(const float value)
	{
		mSamplerInfo.maxAnisotropy = value;
		return *this;
	}

	TextureSampler& TextureSampler::EnableCompare(const VkBool32 enable)
	{
		mSamplerInfo.compareEnable = enable;
		return *this;
	}

	TextureSampler& TextureSampler::AddCompareOp(const VkCompareOp operation)
	{
		mSamplerInfo.compareOp = operation;
		return *this;
	}

	TextureSampler& TextureSampler::AddMinLod(const float lod)
	{
		mSamplerInfo.minLod = lod;
		return *this;
	}

	TextureSampler& TextureSampler::AddMaxLod(const float lod)
	{
		mSamplerInfo.maxLod = lod;
		return *this;
	}

	TextureSampler& TextureSampler::AddBorderColor(const VkBorderColor color)
	{
		mSamplerInfo.borderColor = color;
		return *this;
	}

	TextureSampler& TextureSampler::AddUnnormalizedCoordinates(const VkBool32 value)
	{
		mSamplerInfo.unnormalizedCoordinates = value;
		return *this;
	}

	TextureSampler& TextureSampler::AddNext(const void* next)
	{
		mSamplerInfo.pNext = next;
		return *this;
	}

	void TextureSampler::Configure()
	{
		CHECK_RESULT(vkCreateSampler(LOGICAL_DEVICE, &mSamplerInfo, ALLOCATION_CALLBACK, &mSampler), "RESOURCE CREATION FAILED: TEXTURE SAMPLER");
		RENDERER_DEBUG("RESOURCE CREATED: TEXTURE SAMPLER");
	}

	const VkSampler& TextureSampler::GetVulkanSampler()
	{
		return mSampler;
	}
}