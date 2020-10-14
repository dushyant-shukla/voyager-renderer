#pragma once

#include <string>

struct GLFWwindow;

namespace vr
{
	class InputManager;

	struct WindowProperties
	{
		unsigned int mWidth;
		unsigned int mHeight;
		std::string mTitle;
		InputManager* mInput = nullptr;

		/*
			TODO: Probably a good idea to deserialize these properties from a configuration file.
		*/
		WindowProperties(unsigned int w = 1280, unsigned int h = 720, std::string t = "Voyager Renderer");
	};

	class Window
	{
	public:

		~Window();

		void Shutdown();
		bool ShouldShutdown();
		void Update();

		static Window* InitializeWindow(WindowProperties& properties = WindowProperties(), const std::string& sampleName = "");

		GLFWwindow* GetNativeWindow();

		static inline const int WIDTH = 1280;
		static inline const int HEIGHT = 720;

	private:

		Window(const WindowProperties& properties);

		void Init(const WindowProperties& properties);

	private:

		GLFWwindow* mWindow;
		WindowProperties mWindowProperties;
	};
}
