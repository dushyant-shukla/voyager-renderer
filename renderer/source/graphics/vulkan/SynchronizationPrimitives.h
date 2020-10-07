#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vr
{
	class SynchronizationPrimitives
	{
	public:

		inline static const int MAX_FRAME_DRAWS = 2;

		SynchronizationPrimitives();
		~SynchronizationPrimitives();

		void Create(const VkDevice& device, VkAllocationCallbacks* allocationCallbacks);

		const VkSemaphore& GetImageAvailableSemaphore(const unsigned int& index);
		const VkSemaphore& GetRenderFinishedSemaphore(const unsigned int& index);
		const VkFence& GetDrawFence(const unsigned int& index);

	private:

	private:

		VkDevice mLogicalDevice;
		VkAllocationCallbacks* mAllocationCallbacks;

		std::vector<VkSemaphore> mImageAvailable;
		std::vector<VkSemaphore> mRenderFinished;
		std::vector<VkFence> mDrawFences;
	};
}
