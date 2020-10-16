#pragma once

#include <vulkan/vulkan.h>

namespace vr
{
	class TextureSampler
	{
	public:

		TextureSampler();
		~TextureSampler();

		void CreateDefault();

		TextureSampler& AddSamplerCreateFlags(const VkSamplerCreateFlags flags);
		TextureSampler& AddMagFilter(const VkFilter filter);
		TextureSampler& AddMinFilter(const VkFilter filter);
		TextureSampler& AddSamplerMipmapMode(const VkSamplerMipmapMode mode);
		TextureSampler& AddSamplerAddressModeU(const VkSamplerAddressMode mode);
		TextureSampler& AddSamplerAddressModeV(const VkSamplerAddressMode mode);
		TextureSampler& AddSamplerAddressModeW(const VkSamplerAddressMode mode);
		TextureSampler& AddMipLodBias(const float bias);
		TextureSampler& EnableAnistropy(const VkBool32 enable);
		TextureSampler& AddMaxAnistropy(const float value);
		TextureSampler& EnableCompare(const VkBool32 enable);
		TextureSampler& AddCompareOp(const VkCompareOp operation);
		TextureSampler& AddMinLod(const float lod);
		TextureSampler& AddMaxLod(const float lod);
		TextureSampler& AddBorderColor(const VkBorderColor color);
		TextureSampler& AddUnnormalizedCoordinates(const VkBool32 value);
		TextureSampler& AddNext(const void* next);
		void Configure();

		const VkSampler& GetVulkanSampler();

	private:

		VkSamplerCreateInfo mSamplerInfo;

		VkSampler mSampler = VK_NULL_HANDLE;
	};
}
