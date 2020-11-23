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
		WindowProperties(unsigned int w = 1920, unsigned int h = 1080, std::string t = "Voyager Graphics Renderer");
		//WindowProperties(unsigned int w = 1366, unsigned int h = 768, std::string t = "Voyager Graphics Renderer");
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

		static inline const int WIDTH = 1920;
		static inline const int HEIGHT = 1080;
		//static inline const int WIDTH = 1366;
		//static inline const int HEIGHT = 768;

	private:

		Window(const WindowProperties& properties);

		void Init(const WindowProperties& properties);

	private:

		GLFWwindow* mWindow;
		WindowProperties mWindowProperties;
	};
}
