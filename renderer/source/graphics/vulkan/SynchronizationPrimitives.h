#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vr
{
	class SynchronizationPrimitives
	{
	public:

		SynchronizationPrimitives(const VkPhysicalDevice& physicalDevice, const VkDevice& device, VkAllocationCallbacks* allocationCallbacks);
		~SynchronizationPrimitives();

		void Create();

		VkSemaphore GetImageAvailableSemaphore(const unsigned int& index);
		VkSemaphore GetRenderFinishedSemaphore(const unsigned int& index);
		VkFence GetDrawFence(const unsigned int& index);

	private:

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		std::vector<VkSemaphore> mImageAvailable;
		std::vector<VkSemaphore> mRenderFinished;
		std::vector<VkFence> mDrawFences;

		inline static const int MAX_FRAME_DRAWS = 2;
	};
}
