#include "EditingModeCamera.h"
#include "input/InputManager.h"
#include "window/Window.h"
#include "logging/LoggingCore.h"

namespace vr
{
	EditingModeCamera::EditingModeCamera()
	{
		input = InputManager::GetInstance();
	}

	EditingModeCamera::~EditingModeCamera() {}

	void EditingModeCamera::UpdateCameraView()
	{
		glm::mat4 rotM = glm::mat4(1.0f);
		glm::mat4 transM;

		rotM = glm::rotate(rotM, glm::radians(orientation.rotation.x * (matrices.flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(orientation.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(orientation.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 translation = orientation.position;
		if (matrices.flipY)
		{
			translation.y *= -1.0f;
		}
		transM = glm::translate(glm::mat4(1.0f), translation);

		if (type == Type::first_person)
		{
			matrices.view = rotM * transM;
		}
		else
		{
			matrices.view = transM * rotM;
		}

		orientation.viewPosition = glm::vec4(orientation.position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
	}

	bool EditingModeCamera::IsMoving()
	{
		return false;
	}

	void EditingModeCamera::SetRotation(glm::vec3 rotation)
	{
		orientation.rotation = rotation;
		UpdateCameraView();
	}

	void EditingModeCamera::SetPosition(glm::vec3 position)
	{
		orientation.position = position;
		UpdateCameraView();
	}

	void EditingModeCamera::SetPerspective(float radians, float aspect, float zn, float zf)
	{
		fov = radians;
		znear = zn;
		zfar = zf;
		matrices.projection = glm::perspective(radians, aspect, znear, zfar);
		if (matrices.flipY)
		{
			matrices.projection[1][1] *= -1.0f;
		}
	}

	void EditingModeCamera::SetFlipY(bool flipY)
	{
		matrices.flipY = flipY;
	}

	void EditingModeCamera::Update(float frametime)
	{
		// handle zoom // 1338
		{
			glm::vec2 offset = input->GetScrollOffset();
			orientation.position += glm::vec3(0.0f, 0.0f, offset.y * 0.05);
			oldScrollOffset = offset;
		}

		// 1344
		bool eventHadled = false;
		glm::vec2 newMousePosition = input->GetCursorPosition();
		glm::vec2 delta = oldMousePosition - newMousePosition;
		//left button
		{
			if (input->IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				glm::vec2 d = SanityCheck(delta, 10.0f);
				glm::vec3 rotation = glm::vec3(d.y * speed.rotation, -d.x * speed.rotation, 0.0f);
				orientation.rotation += rotation;
				eventHadled = true;
			}
		}

		// right button
		{
			if (input->IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			{
				//RENDERER_CRITICAL("DELTA: {0}, {1}", delta.x, delta.y);
				glm::vec2 d = SanityCheck(delta, 10.0f);
				glm::vec3 translate = glm::vec3(-d.x * speed.movement, -d.y * speed.movement, 0.0f);
				orientation.position += translate;
				eventHadled = true;
			}
		}

		UpdateCameraView();
		if (eventHadled)
		{
			oldMousePosition = newMousePosition;
		}
	}

	glm::vec2 EditingModeCamera::SanityCheck(const glm::vec2& delta, const float& threshold)
	{
		glm::vec2 d = delta;
		int sign = 1;
		if (delta.x < 0)
		{
			sign = -1;
		}
		if (abs(delta.x) > threshold)
		{
			d.x = sign * threshold;
		}

		sign = 1;
		if (delta.y < 0)
		{
			sign = -1;
		}
		if (abs(delta.y) > threshold)
		{
			d.y = sign * threshold;
		}

		return d;
	}
}