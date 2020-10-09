#include "DescriptorPool.h"
#include "utility/RendererCoreUtility.h"

namespace vr
{
	DescriptorPool::DescriptorPool()
	{
	}

	DescriptorPool::~DescriptorPool()
	{
		if (mPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(mLogicalDevice, mPool, mAllocationCallbacks);
			RENDERER_DEBUG("RESOURCE DESTROYED: DESCRIPTOR POOL");
		}
	}

	DescriptorPool& DescriptorPool::Initialize(const VkDevice& device, VkAllocationCallbacks* allocationCallbacks)
	{
		mLogicalDevice = device;
		mAllocationCallbacks = allocationCallbacks;

		return *this;
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

		CHECK_RESULT(vkCreateDescriptorPool(mLogicalDevice, &createInfo, mAllocationCallbacks, &mPool), "RESOURCE CREATION FAILED: DESCRIPTOR POOL");
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