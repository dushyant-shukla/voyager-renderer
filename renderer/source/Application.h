#pragma once

#include <memory>
#include <window/Window.h>
#include <vulkan/vulkan.h>

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
		~Application() = default;

		void Run();

		void Render();

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

		/*
			wait for device to clean up
		*/
		void Wait();

	protected:

		Instance* mInstance = { nullptr };
		Surface* mSurface = { nullptr };
		Device* mDevice = { nullptr };
		Swapchain* mSwapchain = { nullptr };

	private:

		std::unique_ptr<Window> mWindow;
		std::string mName;
	};

	Application* CreateApplication();
}
