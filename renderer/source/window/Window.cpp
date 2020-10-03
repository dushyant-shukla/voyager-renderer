#include <GLFW/glfw3.h>

#include "Window.h"
#include "assertions.h"
#include "input/InputManager.h"

namespace vr
{
	WindowProperties::WindowProperties(unsigned int w, unsigned int h, std::string t)
		:mWidth{ w }, mHeight{ h }, mTitle{ t }
	{
		mInput = InputManager::GetInstance();
		mInput->SetMouseMaxPositions(w, h);
	}

	Window::Window(const WindowProperties& properties)
	{
		Init(properties);
	}

	vr::Window::~Window() { Shutdown(); }

	void Window::Shutdown()
	{
		glfwDestroyWindow(mWindow);
	}

	bool Window::ShouldShutdown()
	{
		return glfwWindowShouldClose(mWindow);
	}

	void Window::Update()
	{
		glfwPollEvents();
	}

	Window* Window::InitializeWindow(const WindowProperties& properties)
	{
		return new Window(properties);
	}

	GLFWwindow* Window::GetNativeWindow()
	{
		return mWindow;
	}

	void Window::Init(const WindowProperties& properties)
	{
		mWindowProperties = { properties };

		ASSERT_SUCCESS_AND_THROW(glfwInit(), "Failed to initialize GLFW.");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		mWindow = glfwCreateWindow(properties.mWidth, properties.mHeight, properties.mTitle.c_str(), nullptr, nullptr);
		ASSERT_SUCCESS_AND_THROW(mWindow, "Failed to create window.");

		glfwSetWindowUserPointer(mWindow, &mWindowProperties);

		// Key callback
		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scanCode, int action, int mods) {
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);

			switch (action) {
			case GLFW_PRESS:
			{
				properties.mInput->UpdateKeyboardState(key, true, true);
				break;
			}

			case GLFW_RELEASE:
			{
				properties.mInput->UpdateKeyboardState(key, false, false);
				break;
			}
			}
			});

		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
			switch (action) {
			case GLFW_PRESS:
			{
				properties.mInput->UpdateMouseButtonState(button, true);
				break;
			}

			case GLFW_RELEASE:
			{
				properties.mInput->UpdateMouseButtonState(button, false);
				break;
			}
			default:
				break;
			}
			});

		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double positionX, double positionY) {
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
			properties.mInput->UpdateCursorPosition(positionX, positionY);
			});

		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height)
			{
				WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
				properties.mWidth = width;
				properties.mHeight = height;

				//Renderer2D::OnWindowResize(width, height);
				// TODO: window resize callback
			});

		RENDERER_DEBUG("Window initialization successful.");
	}
}