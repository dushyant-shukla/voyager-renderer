#pragma once

#include <memory>
#include <window/Window.h>
#include <vulkan/vulkan.h>

namespace vr
{
	class Instance;
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

	private:

		std::unique_ptr<Window> mWindow;
		std::string mName;
	};

	Application* CreateApplication();
}
