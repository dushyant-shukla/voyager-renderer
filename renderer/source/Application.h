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

		virtual void Draw() = 0;
		virtual void InitializeScene() = 0;
		virtual void SetupPipeline() = 0;

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

		virtual void CleanupScene() = 0;

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
		Framebuffers mFramebuffers;
		CommandBuffers mGraphicsCommandBuffers;
		CommandPool mTransferCommandPool;
		SynchronizationPrimitives mSynchronizationPrimitives;

	private:

		std::unique_ptr<Window> mWindow;
		std::string mName;
	};

	Application* CreateApplication();
}
