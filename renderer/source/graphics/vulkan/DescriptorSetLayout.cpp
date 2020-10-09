#include "DescriptorSetLayout.h"
#include "utility/RendererCoreUtility.h"

namespace vr
{
	DescriptorSetLayout::DescriptorSetLayout()
	{
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(mLogicalDevice, mLayout, mAllocationCallbacks);
		RENDERER_DEBUG("RESOURCE DESTROYED: DESCRIPTOR SET LAYOUT");
	}

	DescriptorSetLayout& DescriptorSetLayout::AddLayoutBinding(unsigned int binding, VkDescriptorType descriptorType, unsigned int descriptorCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers)
	{
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = binding;											// binding of mvp uniform in vertex shader
		layoutBinding.descriptorType = descriptorType;		// type of descriptor (UNIFORM, DYNAMIC UNIFORM etc)
		layoutBinding.descriptorCount = descriptorCount;									// number of descriptors for binding
		layoutBinding.stageFlags = stageFlags;				// shader stage to bind to
		layoutBinding.pImmutableSamplers = pImmutableSamplers;							// for texture: can make sampler data immutable by specifying a layout

		mLayoutBindings.push_back(layoutBinding);

		return *this;
	}

	void DescriptorSetLayout::Create(VkDevice logicalDevice, VkAllocationCallbacks* allocationCallbacks, VkDescriptorSetLayoutCreateFlags flags, void* next)
	{
		mLogicalDevice = logicalDevice;
		mAllocationCallbacks = allocationCallbacks;

		VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = static_cast<unsigned int>(mLayoutBindings.size());
		layoutCreateInfo.pBindings = mLayoutBindings.empty() ? nullptr : mLayoutBindings.data();
		layoutCreateInfo.flags = flags;
		layoutCreateInfo.pNext = next;

		CHECK_RESULT(vkCreateDescriptorSetLayout(mLogicalDevice, &layoutCreateInfo, mAllocationCallbacks, &mLayout), "RESOURCE CREATION FAILED: DESCRIPTOR SET LAYOUT");

		RENDERER_DEBUG("RESOURCE CREATED: DESCRIPTOR SET LAYOUT");
	}

	const VkDescriptorSetLayout& DescriptorSetLayout::GetVkDescriptorSetLayout()
	{
		return mLayout;
	}
}