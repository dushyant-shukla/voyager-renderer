#pragma once

#include <memory>
#include <window/Window.h>

namespace vr
{
	class Application
	{
	public:

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

	private:

		std::unique_ptr<Window> mWindow;
	};

	Application* CreateApplication();
}
