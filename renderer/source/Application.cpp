#include "Application.h"
#include "logging/Logger.h"

namespace vr
{
	void Application::Run()
	{
		Logger::Init();
		InitializeWindow();
		InitializeRenderer();
		InitializeScene();
		Render();
		CleanupSystem();
	}

	void Application::Render()
	{
		while (!mWindow->ShouldShutdown())
		{
			mWindow->Update();
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
	}

	void Application::Wait()
	{
	}
}