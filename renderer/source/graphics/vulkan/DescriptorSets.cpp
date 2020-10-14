#include "DescriptorSets.h"
#include "utility/RendererCoreUtility.h"
#include "RendererState.h"
#include "assertions.h"

namespace vr
{
	DescriptorSets::DescriptorSets() {}

	/*
		No need for deallocation as descriptor sets are destroyed as soon as the corresponding
		pool is freed.
	*/
	DescriptorSets::~DescriptorSets() {}

	void DescriptorSets::Setup(VkDescriptorSetLayout layout, VkDescriptorPool pool, unsigned int count)
	{
		mSets.resize(count);
		std::vector<VkDescriptorSetLayout> layouts(count, layout);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = count;
		allocInfo.pSetLayouts = layouts.data();

		CHECK_RESULT(vkAllocateDescriptorSets(LOGICAL_DEVICE, &allocInfo, mSets.data()), "RESOURCE ALLOCATION FAILED: DESCRIPTOR SETS");
		RENDERER_DEBUG("RESOURCE ALLOCATED: DESCRIPTOR SET");
	}

	const VkDescriptorSet& DescriptorSets::operator[](unsigned int index)
	{
		if (index >= mSets.size())
		{
			THROW("INVALID INDEX USED FOR ACCESSING DESCRIPTOR SET");
		}
		return mSets[index];
	}
}