#include "Application.h"
#include "logging/LoggingCore.h"
#include "frame-rate-controller/FramerateController.h"
#include "graphics/vulkan/Instance.h"
#include "graphics/vulkan/Surface.h"
#include "graphics/vulkan/Device.h"
#include "graphics/vulkan/Swapchain.h"
#include "graphics/vulkan/utility/ImageUtility.h"
#include "graphics/vulkan/utility/MemoryUtility.h"
#include "RendererState.h"

namespace vr
{
	Application::Application(std::string name) :
		mName(name),
		mDepthBuffer()
	{
		mInputManager = InputManager::GetInstance();
	}

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
			Wait();
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
			mCamera.Update(framerateController->GetFrameTime());
			eCamera.Update(framerateController->GetFrameTime());
			Draw();
			mInputManager->LateUpdate(framerateController->GetFrameTime());
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

		// NOTE:: DO NOT CHANGE THE STATE INITIALIZATION ORDER
		RendererState::SetLogicalDevice(mDevice->GetLogicalDevice().device);
		RendererState::SetPhysicalDevice(mDevice->GetPhysicalDevice().device);
		RendererState::SetAllocationCallbacks(nullptr);

		mSwapchain = Swapchain::CreateSwapchain(mDevice, mSurface, mWindow->GetNativeWindow(), nullptr);

		mDepthBuffer.CreateDefault(mSwapchain->GetSwapchainExtent());
		mRenderpass.SetupDefaultRenderPass(mSwapchain->GetSurfaceFormat(), mDepthBuffer.GetFormat());
		mFramebuffers.Create(mSwapchain->GetSwapchainImages(), mSwapchain->GetSwapchainExtent(), mDepthBuffer.GetImageView(), mRenderpass.GetVulkanRenderPass());
		mTransferCommandPool.Create(mDevice->GetPhysicalDevice().queueFamilies.transfer.value(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		mGraphicsCommandBuffers.Create(mDevice->GetPhysicalDevice().queueFamilies.graphics.value(), mSwapchain->GetSwapchainImages().size());
		mSyncPrimitives.Create();

		RendererState::SetGraphicsCommandPool(mGraphicsCommandBuffers.mCommandPool);
		RendererState::SetGraphicsQueue(mDevice->GetLogicalDevice().graphicsQueue);
		RendererState::SetTransferCommandPool(mTransferCommandPool.GetVulkanCommandPool());
		RendererState::SetTransferQueue(mDevice->GetLogicalDevice().transferQueue);
	}

	void Application::Wait()
	{
		vkDeviceWaitIdle(mDevice->GetLogicalDevice().device);
	}
}