#include "Application.h"
#include "logging/LoggingCore.h"
#include "frame-rate-controller/FramerateController.h"
#include "graphics/vulkan/Instance.h"
#include "graphics/vulkan/Surface.h"
#include "graphics/vulkan/Device.h"
#include "graphics/vulkan/Swapchain.h"

namespace vr
{
	Application::Application(std::string name) : mName(name) {}

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
		FramerateController* framerateController = FramerateController::GetInstance(60); // TODO: read this from configuration file
		while (!mWindow->ShouldShutdown())
		{
			framerateController->FrameStart();
			mWindow->Update();
			// TODO call scene render()
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
		mSwapchain = Swapchain::CreateSwapchain(mDevice, mSurface, mWindow->GetNativeWindow(), nullptr);
	}

	void Application::Wait()
	{
	}
}