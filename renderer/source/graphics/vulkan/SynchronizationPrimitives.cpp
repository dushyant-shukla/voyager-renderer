#include "SynchronizationPrimitives.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"
#include "assertions.h"

namespace vr
{
	SynchronizationPrimitives::SynchronizationPrimitives()
	{
	}

	SynchronizationPrimitives::~SynchronizationPrimitives()
	{
		if (!mRenderFinished.empty())
		{
			for (size_t i = 0; i < MAX_FRAME_DRAWS; ++i)
			{
				vkDestroySemaphore(LOGICAL_DEVICE, mRenderFinished[i], ALLOCATION_CALLBACK);
				vkDestroySemaphore(LOGICAL_DEVICE, mImageAvailable[i], ALLOCATION_CALLBACK);
				vkDestroyFence(LOGICAL_DEVICE, mDrawFences[i], ALLOCATION_CALLBACK);
			}

			RENDERER_DEBUG("RESOURCES DESTROYED: SYNCHRONIZATION PRIMITIVES");
		}
	}

	void SynchronizationPrimitives::Create()
	{
		mImageAvailable.resize(MAX_FRAME_DRAWS);
		mRenderFinished.resize(MAX_FRAME_DRAWS);
		mDrawFences.resize(MAX_FRAME_DRAWS);

		// semaphore creation information
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// fence creation information
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;		// we want this fence to signal (open) when created

		for (size_t i = 0; i < MAX_FRAME_DRAWS; ++i)
		{
			CHECK_RESULT(vkCreateSemaphore(LOGICAL_DEVICE, &semaphoreCreateInfo, ALLOCATION_CALLBACK, &mImageAvailable[i]), "RESOURCE CREATION FAILED: IMAGE AVAILABLE SEMAPHORE");
			CHECK_RESULT(vkCreateSemaphore(LOGICAL_DEVICE, &semaphoreCreateInfo, ALLOCATION_CALLBACK, &mRenderFinished[i]), "RESOURCE CREATION FAILED: RENDER FINISHED SEMAPHORE");
			CHECK_RESULT(vkCreateFence(LOGICAL_DEVICE, &fenceCreateInfo, ALLOCATION_CALLBACK, &mDrawFences[i]), "RESOURCE CREATION FAILED: DRAW FENCES SEMAPHORE");
		}

		RENDERER_DEBUG("RESOURCES CREATED: SYNCHRONIZATION PRIMITIVES");
	}

	const VkSemaphore& SynchronizationPrimitives::GetImageAvailableSemaphore(const unsigned int& index)
	{
		if (index >= MAX_FRAME_DRAWS)
		{
			THROW("INVALID INDEX USED FOR ACCESSING IMAGE AVAILABLE SEMAPHORE");
		}
		return mImageAvailable[index];
	}

	const VkSemaphore& SynchronizationPrimitives::GetRenderFinishedSemaphore(const unsigned int& index)
	{
		if (index >= MAX_FRAME_DRAWS)
		{
			THROW("INVALID INDEX USED FOR ACCESSING RENDER FINISHED SEMAPHORE");
		}
		return mRenderFinished[index];
	}

	const VkFence& SynchronizationPrimitives::GetDrawFence(const unsigned int& index)
	{
		if (index >= MAX_FRAME_DRAWS)
		{
			THROW("INVALID INDEX USED FOR ACCESSING DRAW FENCES");
		}
		return mDrawFences[index];
	}
}