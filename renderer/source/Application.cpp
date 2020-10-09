#include "Application.h"
#include "logging/LoggingCore.h"
#include "frame-rate-controller/FramerateController.h"
#include "graphics/vulkan/Instance.h"
#include "graphics/vulkan/Surface.h"
#include "graphics/vulkan/Device.h"
#include "graphics/vulkan/Swapchain.h"
#include "graphics/vulkan/utility/ImageUtility.h"
#include "graphics/vulkan/utility/MemoryUtility.h"

namespace vr
{
	Application::Application(std::string name) :
		mName(name),
		mDepthBuffer()
	{}

	Application::~Application()
	{}

	void Application::Run()
	{
		try
		{
			Logger::Init();
			InitializeWindow();
			InitializeRenderer();
			InitializeScene();
			Render();
			CleanupScene();
			CleanupSystem();
		}
		catch (std::runtime_error& error)
		{
			RENDERER_CRITICAL(error.what());
		}
	}

	void Application::Render()
	{
		FramerateController* framerateController = FramerateController::GetInstance(60); // TODO: read this from configuration file
		while (!mWindow->ShouldShutdown())
		{
			framerateController->FrameStart();
			mWindow->Update();
			Draw();
			framerateController->FrameEnd();
		}

		Wait();
	}

	VkPhysicalDeviceFeatures Application::CheckRequiredFeatures()
	{
		return mDevice->GetPhysicalDevice().features;
	}

	void Application::InitializeWindow()
	{
		mWindow = std::unique_ptr<Window>(Window::InitializeWindow(WindowProperties(), mName));
	}

	void Application::CleanupSystem()
	{
	}

	void Application::InitializeRenderer()
	{
		mInstance = Instance::CreateInstance(mName);
		mSurface = Surface::CreateWindowSurface(mInstance, mWindow->GetNativeWindow(), nullptr);

		mDevice = Device::InitializeDevice(mInstance, mSurface);
		mDevice->SetupLogicalDevice(CheckRequiredFeatures());

		VkDevice logicalDevice = mDevice->GetLogicalDevice().device;
		VkPhysicalDevice physicalDevice = mDevice->GetPhysicalDevice().device;

		// initializing utilities
		ImageUtility::sLogicalDevice = logicalDevice;

		MemoryUtility::sLogicalDevice = logicalDevice;
		MemoryUtility::sPhysicalDevice = physicalDevice;

		mSwapchain = Swapchain::CreateSwapchain(mDevice, mSurface, mWindow->GetNativeWindow(), nullptr);

		mDepthBuffer.CreateDefault(mDevice->GetPhysicalDevice().device, mDevice->GetLogicalDevice().device, nullptr, mSwapchain->GetSwapchainExtent());
		mRenderpass.SetupDefaultRenderPass(logicalDevice, nullptr, mSwapchain->GetSurfaceFormat(), mDepthBuffer.GetFormat());
		mFramebuffers.Create(logicalDevice, nullptr, mSwapchain->GetSwapchainImages(), mSwapchain->GetSwapchainExtent(), mDepthBuffer.GetImageView(), mRenderpass.GetVulkanRenderPass());
		mTransferCommandPool.Create(logicalDevice, nullptr, mDevice->GetPhysicalDevice().queueFamilies.transfer.value(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		mGraphicsCommandBuffers.Create(logicalDevice, nullptr, mDevice->GetPhysicalDevice().queueFamilies.graphics.value(), mSwapchain->GetSwapchainImages().size());
		mSynchronizationPrimitives.Create(logicalDevice, nullptr);
	}

	void Application::Wait()
	{
		vkDeviceWaitIdle(mDevice->GetLogicalDevice().device);
	}
}