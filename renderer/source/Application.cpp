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

#include <imgui.h>

namespace vr
{
	Application::Application(std::string name) :
		mName(name),
		mDepthBuffer()
	{
		mInputManager = InputManager::GetInstance();
	}

	Application::~Application()
	{
		if (mUiOverlay.active)
		{
			mUiOverlay.mUI.FreeResources();
		}
	}

	void Application::Run()
	{
		try
		{
			Logger::Init();
			InitializeWindow();
			InitializeRenderer();
			InitializeUI();
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
			Draw(framerateController->GetFrameTime());
			mInputManager->LateUpdate(framerateController->GetFrameTime());
			framerateController->FrameEnd();
			UpdateUI(framerateController->GetFrameTime());
			eCamera.Update(framerateController->GetFrameTime());
		}

		Wait();
	}

	VkPhysicalDeviceFeatures Application::CheckRequiredFeatures()
	{
		return mDevice->GetPhysicalDevice().features;
	}

	void Application::OnUpdateUIOverlay(UiOverlay* overlay)
	{
	}

	/*
		This method must be called from Record commands method in sub-classes.
	*/
	void Application::DrawUI(const VkCommandBuffer commandBuffer)
	{
		if (mUiOverlay.active)
		{
			mUiOverlay.mUI.Draw(commandBuffer);
		}
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
		mPipelineCache.Setup();
		mFramebuffers.Create(mSwapchain->GetSwapchainImages(), mSwapchain->GetSwapchainExtent(), mDepthBuffer.GetImageView(), mRenderpass.GetVulkanRenderPass());
		mTransferCommandPool.Create(mDevice->GetPhysicalDevice().queueFamilies.transfer.value(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		mGraphicsCommandBuffers.Create(mDevice->GetPhysicalDevice().queueFamilies.graphics.value(), mSwapchain->GetSwapchainImages().size());
		mSyncPrimitives.Create();

		RendererState::SetGraphicsCommandPool(mGraphicsCommandBuffers.mCommandPool);
		RendererState::SetGraphicsQueue(mDevice->GetLogicalDevice().graphicsQueue);
		RendererState::SetTransferCommandPool(mTransferCommandPool.GetVulkanCommandPool());
		RendererState::SetTransferQueue(mDevice->GetLogicalDevice().transferQueue);
	}

	void Application::InitializeUI()
	{
		mUiOverlay.mUI.PrepareResources();
		mUiOverlay.mUI.PreparePipeline(mPipelineCache.mCache, mRenderpass.GetVulkanRenderPass(), mSwapchain->GetSwapchainExtent());
	}

	void Application::UpdateUI(const double& frametime)
	{
		if (!mUiOverlay.active)
		{
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)Window::WIDTH, (float)Window::HEIGHT);
		io.DeltaTime = frametime;

		glm::vec2 mousePosition = mInputManager->GetCursorPosition();
		io.MousePos = ImVec2(mousePosition.x, mousePosition.y);
		io.MouseDown[0] = mInputManager->IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
		io.MouseDown[1] = mInputManager->IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

		ImGui::NewFrame();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding | ImGuiStyleVar_FrameRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Voyager Graphics Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::TextUnformatted(mDevice->GetPhysicalDevice().properties.deviceName);
		if (mUiOverlay.mUI.CheckBox("Enable frame-rate controller", &FramerateController::mControlFramerate))
		{
		}

		/*
			frame rate plotting
		*/
		{
			std::rotate(mUiOverlay.frameTimes.begin(), mUiOverlay.frameTimes.begin() + 1, mUiOverlay.frameTimes.end());
			float frameTime = 1000.0f / (frametime * 1000.0f);
			mUiOverlay.frameTimes.back() = frameTime;
			if (frameTime < mUiOverlay.frameTimeMin) {
				mUiOverlay.frameTimeMin = frameTime;
			}
			if (frameTime > mUiOverlay.frameTimeMax) {
				mUiOverlay.frameTimeMax = frameTime;
			}

			ImGui::PlotLines("frame rate", &mUiOverlay.frameTimes[0], 50, 0, "", mUiOverlay.frameTimeMin, mUiOverlay.frameTimeMax, ImVec2(0, 80), 4);
		}
		// frame rate plotting ends

		ImGui::PushItemWidth(110.0f * mUiOverlay.mUI.mUiState.scale);

		/*
			Call to application specific UI elements
		*/
		OnUpdateUIOverlay(&mUiOverlay.mUI);

		ImGui::PopItemWidth();

		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::Render();

		if (mUiOverlay.mUI.Update() || mUiOverlay.mUI.mUiState.updated)
		{
			//buildCommandBuffers();
			mUiOverlay.mUI.mUiState.updated = false;
		}
	}

	void Application::Wait()
	{
		vkDeviceWaitIdle(mDevice->GetLogicalDevice().device);
	}
}