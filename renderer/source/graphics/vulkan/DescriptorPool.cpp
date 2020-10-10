#include "DescriptorPool.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"

namespace vr
{
	DescriptorPool::DescriptorPool()
	{
	}

	DescriptorPool::~DescriptorPool()
	{
		if (mPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(LOGICAL_DEVICE, mPool, ALLOCATION_CALLBACK);
			RENDERER_DEBUG("RESOURCE DESTROYED: DESCRIPTOR POOL");
		}
	}

	/*
		A pool for all descriptors in a descriptor set
	*/
	void DescriptorPool::Create(VkDescriptorPoolCreateFlags flags, unsigned int maxSets, void* next)
	{
		VkDescriptorPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = mPoolSizes.empty() ? 0 : mPoolSizes.size();
		createInfo.pPoolSizes = mPoolSizes.empty() ? nullptr : mPoolSizes.data();
		createInfo.maxSets = maxSets;
		createInfo.flags = flags;
		createInfo.pNext = next;

		CHECK_RESULT(vkCreateDescriptorPool(LOGICAL_DEVICE, &createInfo, ALLOCATION_CALLBACK, &mPool), "RESOURCE CREATION FAILED: DESCRIPTOR POOL");
		RENDERER_DEBUG("RESOURCE CREATED: DESCRIPTOR POOL");
	}

	/*
		Pool size for each descriptor in the descriptor set
	*/
	DescriptorPool& DescriptorPool::AddPoolSize(const VkDescriptorType type, unsigned int count)
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.descriptorCount = type;
		poolSize.descriptorCount = count;
		mPoolSizes.push_back(poolSize);
		return *this;
	}

	const VkDescriptorPool& DescriptorPool::GetVulkanDescriptorPool()
	{
		return mPool;
	}
}