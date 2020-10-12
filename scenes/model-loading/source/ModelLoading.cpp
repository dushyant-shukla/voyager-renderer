#include "ModelLoading.h"

namespace vr
{
	ModelLoading::ModelLoading(std::string name) : Application(name)
	{
	}

	ModelLoading::~ModelLoading()
	{
	}
	void ModelLoading::InitializeScene()
	{
	}
	void ModelLoading::CleanupScene()
	{
	}
	void ModelLoading::Draw()
	{
	}
	VkPhysicalDeviceFeatures ModelLoading::CheckRequiredFeatures()
	{
		bool requiredFeaturesAvailable = true;
		VkPhysicalDeviceFeatures requiredDeviceFeatures = {};
		if (!mDevice->GetPhysicalDevice().features.samplerAnisotropy)
		{
			requiredFeaturesAvailable = false;
		}

		if (requiredFeaturesAvailable)
		{
			requiredDeviceFeatures.samplerAnisotropy = VK_TRUE;
			return requiredDeviceFeatures;
		}

		throw std::runtime_error("DEVICE NOT SUITABLE: REQUIRED FEATURES NOT AVAILABLE");
	}

	Application* CreateApplication()
	{
		return new ModelLoading("Model Loading");
	}
}