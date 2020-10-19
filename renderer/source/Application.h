#pragma once

#include <memory>
#include <window/Window.h>
#include <vulkan/vulkan.h>
#include "graphics/vulkan/DepthBuffer.h"
#include "graphics/vulkan/RenderPass.h"
#include "graphics/vulkan/Framebuffers.h"
#include "graphics/vulkan/CommandBuffers.h"
#include "graphics/vulkan/CommandPool.h"
#include "graphics/vulkan/SynchronizationPrimitives.h"
#include "graphics/vulkan/PipelineCache.h"
#include "camera/Camera.h"
#include "input/InputManager.h"
#include "camera/EditingModeCamera.h"
#include "ui/UiOverlay.h"

namespace vr
{
	class Instance;
	class Surface;
	class Device;
	class Swapchain;

	/*
		Base class inherited by all samples.
		It acts as an interface between the samples and the entry point to the renderer.
	*/
	class Application
	{
	public:

		Application(std::string name);
		virtual ~Application();

		void Run();

		void Render();

		virtual void Draw(const double& frametime) = 0;
		virtual void InitializeScene() = 0;
		virtual VkPhysicalDeviceFeatures CheckRequiredFeatures();
		virtual void OnUpdateUIOverlay(UiOverlay* overlay);
		virtual void CleanupScene() = 0;
		void DrawUI(const VkCommandBuffer commandBuffer);

	private:

		/*
			Creates windows to render on.
			Called from Application::InitializeSystem().
		*/
		void InitializeWindow();

		/*
			Cleans resources allocated by renderer.
			The projects (samples applications) should take care of freeing the resources they create.
			Sample applications must call this method before cleaning up their resources.
		*/
		void CleanupSystem();

		/*
			Initiates rendering API
		*/
		void InitializeRenderer();

		void InitializeUI();
		void UpdateUI(const double& frametime);

		/*
			wait for device to clean up
		*/
		void Wait();

	protected:

		Instance* mInstance = { nullptr };
		Surface* mSurface = { nullptr };
		Device* mDevice = { nullptr };
		Swapchain* mSwapchain = { nullptr };
		DepthBuffer mDepthBuffer;
		RenderPass mRenderpass;
		PipelineCache mPipelineCache;
		Framebuffers mFramebuffers;
		CommandBuffers mGraphicsCommandBuffers;
		CommandPool mTransferCommandPool;
		SynchronizationPrimitives mSyncPrimitives;

		// Stores the features available on the selected physical device
		VkPhysicalDeviceFeatures deviceFeatures;

		// Set of physical device features to be enabled for this example (must be set in the derived constructor)
		VkPhysicalDeviceFeatures enabledFeatures{};

		bool isReady = false;

		Camera mCamera;
		EditingModeCamera eCamera;
		InputManager* mInputManager;

		struct
		{
			UiOverlay mUI;
			bool active = true;
			std::array<float, 50> frameTimes{};
			float frameTimeMin = 9999.0f;
			float frameTimeMax = 0.0f;
		} mUiOverlay;

	private:

		std::unique_ptr<Window> mWindow;
		std::string mName;
	};

	Application* CreateApplication();
}
