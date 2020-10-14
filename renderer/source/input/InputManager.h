#pragma once

#include <glm/glm.hpp>

#include "MouseButton.h"
#include "KeyboardKeys.h"

namespace vr {
	class InputManager {
	public:

		static InputManager* GetInstance();
		~InputManager();

		void Update(double deltaTime);
		void LateUpdate(double deltaTime);

		void UpdateKeyboardState(const int& key, const bool& state, const bool& repeat);
		bool IsKeyPressed(const int& keyCode);
		bool IsKeyTriggered(const int& keyCode);
		bool IsKeyReleased(const int& keyCode);

		template<class ...Args>
		bool IsAllKeyNotPressed(const Args&... args)
		{
			bool ret = false;
			std::vector<int> vec = { args... };
			for (unsigned i = 0; i < vec.size(); ++i)
			{
				ret = ret || IsKeyPressed(vec[i]);
			}
			return !ret;
		}

		void UpdateMouseButtonState(const int& button, const bool& state);
		bool IsMouseButtonTriggered(const int& mouseButton);
		bool IsMouseButtonPressed(const int& mouseButton);
		bool IsMouseButtonReleased(const int& mouseButton);

		void UpdateScrollOffset(const double& offsetX, const double& offsetY);
		void UpdateCursorPosition(const double& positionX, const double& positionY);
		glm::vec2 GetScrollOffset();
		const glm::vec2& GetCursorPosition() const;
		const double GetMousePositionX();
		const double GetMousePositionY();
		void SetMouseMaxPositions(double x, double y);
		const glm::vec2& GetCursorMaxPosition() const;
		const double GetMouseMaxPositionX();
		const double GetMouseMaxPositionY();

		const glm::vec2 GetCursorViewPosition(float _x, float _y);
		const glm::vec2 GetCursorViewPosition();
		float lerp(float x, float y, float t);

	private:

		InputManager();

		bool m_previousKeyBoardState[KEY_LAST + 1];
		bool m_currentKeyBoardState[KEY_LAST + 1];

		bool m_previousMouseButtonState[MOUSE_BUTTON_LAST + 1];
		bool m_currentMouseButtonState[MOUSE_BUTTON_LAST + 1];

		glm::vec2 m_cursorPosition;
		glm::vec2 m_cursorMaxPosition;
		glm::vec2 m_scrollOffset;
	};
}
