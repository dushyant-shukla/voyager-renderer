#pragma once
#include <glm/glm.hpp>
#include <assimp/scene.h>

namespace vrmath
{
	class Quaternion
	{
	public:
		Quaternion();
		Quaternion(glm::vec4 rhs);
		~Quaternion();

		Quaternion(glm::mat4 mRotation);
		Quaternion(float x, float y, float z, float s);
		Quaternion(glm::vec3 mAxis, float mAngle);

		Quaternion& operator=(Quaternion const& rhs);
		Quaternion& operator*(float const data);
		Quaternion& operator/(float const data);
		Quaternion& operator+(Quaternion const& rhs);
		Quaternion& operator-(Quaternion const& rhs);
		friend Quaternion& operator* (float const data, Quaternion const& rhs);
		Quaternion& operator* (Quaternion const& rhs);

		glm::mat4 ToMatrix();//rotation matrix
		glm::vec3 Rotate(glm::vec3 x);

		aiMatrix4x4 ToAiRotationMatrix();
		Quaternion Conjugate();
		Quaternion Inverse();

		void Normalize();
		float operator[](unsigned int i);
		float Dot(Quaternion const& rhs);
		glm::vec3 ReturnV() const;
		float ReturnS() const;

		static Quaternion Slerp(Quaternion& A, Quaternion& B, float blend);
		static Quaternion Nlerp(Quaternion a, Quaternion b, float blend);

	private:
		glm::vec4 mQuaternion;
	};
}
