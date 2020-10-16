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
		TextureSampler& MagFilter(const VkFilter filter);
		TextureSampler& MinFilter(const VkFilter filter);
		TextureSampler& MipmapMode(const VkSamplerMipmapMode mode);
		TextureSampler& AddressModeU(const VkSamplerAddressMode mode);
		TextureSampler& AddressModeV(const VkSamplerAddressMode mode);
		TextureSampler& AddressModeW(const VkSamplerAddressMode mode);
		TextureSampler& MipLodBias(const float bias);
		TextureSampler& EnableAnistropy(const VkBool32 enable);
		TextureSampler& MaxAnistropy(const float value);
		TextureSampler& EnableCompare(const VkBool32 enable);
		TextureSampler& CompareOp(const VkCompareOp operation);
		TextureSampler& MinLod(const float lod);
		TextureSampler& MaxLod(const float lod);
		TextureSampler& BorderColor(const VkBorderColor color);
		TextureSampler& UnnormalizedCoordinates(const VkBool32 value);
		TextureSampler& AddNext(const void* next);
		void Configure();

		const VkSampler& GetVulkanSampler();

	private:

		VkSamplerCreateInfo mSamplerInfo;

		VkSampler mSampler = VK_NULL_HANDLE;
	};
}
