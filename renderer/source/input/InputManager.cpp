#include <glm/glm.hpp>

#include "InputManager.h"
#include "assertions.h"

namespace vr {
	InputManager* InputManager::GetInstance() {
		static InputManager input;
		return &input;
	}

	InputManager::InputManager()
		: m_cursorPosition(glm::vec2(0.0f, 0.0f)),
		m_cursorMaxPosition(glm::vec2(0.0f, 0.0f))
	{
		memset(m_previousKeyBoardState, false, sizeof(bool) * (KEY_LAST + 1));
		memset(m_currentKeyBoardState, false, sizeof(bool) * (KEY_LAST + 1));

		memset(m_previousMouseButtonState, false, sizeof(bool) * (MOUSE_BUTTON_LAST + 1));
		memset(m_currentMouseButtonState, false, sizeof(bool) * (MOUSE_BUTTON_LAST + 1));
	}

	InputManager::~InputManager() {
	}

	void InputManager::Update(double deltaTime)
	{
	}

	void InputManager::LateUpdate(double deltaTime)
	{
		for (int i = 0; i < KEY_LAST + 1; ++i)
		{
			m_previousKeyBoardState[i] = m_currentKeyBoardState[i];
		}
		for (int i = 0; i < MOUSE_BUTTON_LAST + 1; ++i)
		{
			m_previousMouseButtonState[i] = m_currentMouseButtonState[i];
		}
	}

	void InputManager::UpdateKeyboardState(const int& key, const bool& state, const bool& repeat) {
		m_currentKeyBoardState[key] = state;
	}

	void InputManager::UpdateMouseButtonState(const int& button, const bool& state) {
		m_currentMouseButtonState[button] = state;
	}

	bool InputManager::IsKeyPressed(const int& keyCode) {
		ASSERT(keyCode > KEY_LAST, "Invalid key-code.");
		return m_currentKeyBoardState[keyCode];
	}

	bool InputManager::IsKeyTriggered(const int& keyCode) {
		ASSERT(keyCode > KEY_LAST, "Invalid key-code.");

		if (m_currentKeyBoardState[keyCode] && !m_previousKeyBoardState[keyCode]) {
			return true;
		}
		return false;
	}

	bool InputManager::IsKeyReleased(const int& keyCode) {
		ASSERT(keyCode > KEY_LAST, "Invalid key-code.");

		if (!m_currentKeyBoardState[keyCode] && m_previousKeyBoardState[keyCode]) {
			return true;
		}
		return false;
	}

	bool InputManager::IsMouseButtonTriggered(const int& mouseButton)
	{
		ASSERT(mouseButton > MOUSE_BUTTON_LAST, "Invalid mouse-button.");
		if (m_currentMouseButtonState[mouseButton] && !m_previousMouseButtonState[mouseButton]) {
			return true;
		}
		return false;
	}

	bool InputManager::IsMouseButtonPressed(const int& mouseButton) {
		ASSERT(mouseButton > MOUSE_BUTTON_LAST, "Invalid mouse-button.");
		return m_currentMouseButtonState[mouseButton];
	}

	bool InputManager::IsMouseButtonReleased(const int& mouseButton) {
		ASSERT(mouseButton > MOUSE_BUTTON_LAST, "Invalid mouse-button.");

		if (!m_currentMouseButtonState[mouseButton] && m_previousMouseButtonState[mouseButton]) {
			return true;
		}
		return false;
	}

	void InputManager::UpdateCursorPosition(const double& positionX, const double& positionY) {
		m_cursorPosition.x = (float)positionX;
		m_cursorPosition.y = (float)positionY;
	}

	void InputManager::UpdateScrollOffset(const double& offsetX, const double& offsetY) {
		m_scrollOffset.x = (float)offsetX;
		m_scrollOffset.y = (float)offsetY;
	}

	const glm::vec2& InputManager::GetCursorPosition() const
	{
		return m_cursorPosition;
	}

	glm::vec2 InputManager::GetScrollOffset()
	{
		// TODO: This is only a quick fix for endless zoom problem. Think of a better solution.
		glm::vec2 temp = glm::vec2(0.0f);
		std::swap(temp, m_scrollOffset);
		return temp;
	}

	const double InputManager::GetMousePositionX()
	{
		return m_cursorPosition.x;
	}

	const double InputManager::GetMousePositionY()
	{
		return m_cursorPosition.y;
	}
	void InputManager::SetMouseMaxPositions(double x, double y)
	{
		m_cursorMaxPosition.x = (float)x;
		m_cursorMaxPosition.y = (float)y;
	}
	const glm::vec2& InputManager::GetCursorMaxPosition() const
	{
		return m_cursorMaxPosition;
	}
	const double InputManager::GetMouseMaxPositionX()
	{
		return m_cursorMaxPosition.x;
	}
	const double InputManager::GetMouseMaxPositionY()
	{
		return m_cursorMaxPosition.y;
	}

	const glm::vec2 InputManager::GetCursorViewPosition(float _x, float _y)
	{
		float a = -m_cursorMaxPosition.x / m_cursorMaxPosition.y;
		float b = -a;

		float x = lerp(a, b, _x / m_cursorMaxPosition.x);
		float y = lerp(1.0f, -1.0f, _y / m_cursorMaxPosition.y);
		return glm::vec2(x, y);
	}

	const glm::vec2 InputManager::GetCursorViewPosition() {
		float a = -m_cursorMaxPosition.x / m_cursorMaxPosition.y;
		float b = -a;

		float x = lerp(a, b, m_cursorPosition.x / m_cursorMaxPosition.x);
		float y = lerp(1.0f, -1.0f, m_cursorPosition.y / m_cursorMaxPosition.y);
		return glm::vec2(x, y);
	}

	float InputManager::lerp(float x, float y, float t) {
		return x * (1.f - t) + y * t;
	}
}