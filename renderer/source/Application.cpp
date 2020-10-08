#include "Application.h"
#include "logging/LoggingCore.h"
#include "frame-rate-controller/FramerateController.h"
#include "graphics/vulkan/Instance.h"
#include "graphics/vulkan/Surface.h"
#include "graphics/vulkan/Device.h"
#include "graphics/vulkan/Swapchain.h"

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
		Logger::Init();
		InitializeWindow();
		InitializeRenderer();
		InitializeScene();
		Render();
		CleanupScene();
		CleanupSystem();
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

	void Application::InitializeWindow()
	{
		mWindow = std::unique_ptr<Window>(Window::InitializeWindow(WindowProperties()));
	}

	void Application::CleanupSystem()
	{
	}

	void Application::InitializeRenderer()
	{
		mInstance = Instance::CreateInstance(mName);
		mSurface = Surface::CreateWindowSurface(mInstance, mWindow->GetNativeWindow(), nullptr);
		mDevice = Device::InitializeDevice(mInstance, mSurface);

		VkDevice logicalDevice = mDevice->GetLogicalDevice().device;
		VkPhysicalDevice physicalDevice = mDevice->GetPhysicalDevice().device;

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