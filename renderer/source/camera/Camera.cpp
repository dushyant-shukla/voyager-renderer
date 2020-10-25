#include "Camera.h"
#include "input/InputManager.h"
#include "window/Window.h"

namespace vr
{
	Camera::Camera()
	{
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		position = glm::vec3(0.0f, 10.0f, -3.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		front = glm::vec3(0.0f, 0.0f, -1.0f);
		speed = 5.0f;
		yaw = -90.0;
		pitch = 0.0;

		fov = 60.0f;
		znear = 0.1f;
		zfar = 1024.0f;

		view = glm::lookAt(position, position + front, up);
		project = glm::perspective(glm::radians(fov), (float)Window::WIDTH / Window::HEIGHT, znear, zfar);
		project[1][1] *= -1;

		mInput = InputManager::GetInstance();

		firstMove = true;
		currentCursorPosition = glm::vec2(0.0f, 0.0f);
	}

	Camera::~Camera()
	{
	}

	void Camera::Update(float deltaTime)
	{
		float adjustedSpeed = speed * deltaTime;

		if (mInput->IsKeyPressed(KEY_W))
		{
			position += adjustedSpeed * front;
		}
		if (mInput->IsKeyPressed(KEY_S))
		{
			position -= adjustedSpeed * front;
		}
		if (mInput->IsKeyPressed(KEY_A))
		{
			position -= glm::normalize(glm::cross(front, up)) * adjustedSpeed;
		}
		if (mInput->IsKeyPressed(KEY_D))
		{
			position += glm::normalize(glm::cross(front, up)) * adjustedSpeed;
		}

		currentCursorPosition = mInput->GetCursorPosition();
		if (firstMove)
		{
			lastCursorPosition = currentCursorPosition;
			firstMove = false;
		}

		float xOffset = currentCursorPosition.x - lastCursorPosition.x;
		float yOffset = lastCursorPosition.y - currentCursorPosition.y;

		const float senstivity = 0.05f;
		xOffset *= senstivity;
		yOffset *= senstivity;

		if (mInput->IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			yaw += xOffset;
			pitch += yOffset;
		}

		pitch = pitch > PITCH_MAX ? PITCH_MAX : pitch;
		pitch = pitch < PITCH_MIN ? PITCH_MIN : pitch;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(direction);

		view = glm::lookAt(position, position + front, up);
		project = glm::perspective(glm::radians(fov), (float)Window::WIDTH / Window::HEIGHT, znear, zfar);
		project[1][1] *= -1;

		lastCursorPosition.x = currentCursorPosition.x;
		lastCursorPosition.y = currentCursorPosition.y;
	}
}