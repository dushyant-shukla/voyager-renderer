#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vr
{
	class InputManager;

	class EditingModeCamera
	{
	public:

		enum Type { look_at, first_person };

		EditingModeCamera();
		~EditingModeCamera();

		void Update(float frametime);
		void UpdateCameraView();
		bool IsMoving();
		void SetRotation(glm::vec3 rotation);
		void SetPosition(glm::vec3 position);
		void SetPerspective(float radians, float aspect, float near, float far);
		void SetFlipY(bool flipY);
		glm::vec2 SanityCheck(const glm::vec2& delta, const float& threshold);

	public:

		Type type = Type::look_at;

		float fov = 45.0f;
		float znear = 0.1f;
		float zfar = 1024.0f;

		struct
		{
			glm::mat4 projection = glm::mat4();
			glm::mat4 view = glm::mat4();
			bool flipY = false;
		} matrices;

		struct
		{
			float rotation = 1.0f;
			float movement = 0.01f;
			float zoom = 5.50f;
		} speed;

		struct
		{
			glm::vec3 rotation = glm::vec3();
			glm::vec3 position = glm::vec3();
			glm::vec4 viewPosition = glm::vec4();
		} orientation;

		InputManager* input;
		glm::vec2 oldMousePosition = glm::vec2(0.0f);
		glm::vec2 oldScrollOffset = glm::vec2(0.0f);

		static inline bool UPDATE_CAMERA = true;
	};
}
