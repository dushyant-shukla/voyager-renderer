#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vr
{
	class InputManager;

	struct Camera
	{
		enum Type { look_at, first_person };

		Type type = Type::look_at;

		float fov;
		float znear, zfar;

		Camera();
		~Camera();
		void UpdateViewMatrix();
		void Update(float deltaTime);

		// Use to adjust mouse rotation speed
		float rotationSpeed = 0.20f;
		// Use to adjust mouse zoom speed
		float zoomSpeed = 5.50f;

		glm::vec3 rotation;
		glm::vec3 position;
		glm::vec3 up;
		glm::vec3 front;
		glm::vec2 currentCursorPosition;
		glm::vec2 lastCursorPosition;
		float speed;
		float yaw;
		float pitch;

		glm::mat4 view;
		glm::mat4 project;

		InputManager* mInput;

		static inline const float PITCH_MAX = 89.0;
		static inline const float PITCH_MIN = -89.0;
		static inline const float FIELD_OF_VIEW_MIN = 1.0f;
		static inline const float FIELD_OF_VIEW_MAX = 45.0f;

		void SetRotation(glm::vec3 rotation);
		void SetPosition(glm::vec3 position);
		void SetPerspective(float radians, float aspect, float near, float far);

	private:

		bool firstMove;
	};
}
