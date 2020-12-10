#include "Quaternion.h"

namespace vrmath
{
	Quaternion::Quaternion() : mQuaternion(0.0f, 0.0f, 0.0f, 1.0f)
	{
	}

	Quaternion::Quaternion(glm::vec4 rhs)
	{
		mQuaternion.x = rhs.x;
		mQuaternion.y = rhs.y;
		mQuaternion.z = rhs.z;
		mQuaternion.w = rhs.w;
	}

	Quaternion::Quaternion(glm::mat4 mRotation)
	{
		float s = sqrt(mRotation[0][0] + mRotation[1][1] + mRotation[2][2]) / 2;
		float x = (mRotation[1][2] - mRotation[2][1]) / (4 * s);
		float y = (mRotation[2][0] - mRotation[0][2]) / (4 * s);
		float z = (mRotation[0][1] - mRotation[1][0]) / (4 * s);
		mQuaternion = glm::vec4(x, y, z, s);
	}

	Quaternion::Quaternion(float x, float y, float z, float s)
	{
		mQuaternion = glm::vec4(x, y, z, s);
	}

	Quaternion::Quaternion(glm::vec3 mAxis, float mAngle)
	{
		mQuaternion = glm::vec4(glm::sin(glm::radians(mAngle) / 2) * mAxis, glm::cos(glm::radians(mAngle) / 2));
	}

	Quaternion& Quaternion::operator*(float const data)
	{
		mQuaternion *= data; return *this;
	}

	Quaternion& Quaternion::operator/(float const data)
	{
		mQuaternion /= data; return *this;
	}

	Quaternion& Quaternion::operator+(Quaternion const& rhs)
	{
		mQuaternion += rhs.mQuaternion; return *this;
	}

	Quaternion& Quaternion::operator-(Quaternion const& rhs)
	{
		mQuaternion -= rhs.mQuaternion; return *this;
	}

	Quaternion& Quaternion::operator*(Quaternion const& rhs)
	{
		// TODO: insert return statement here
		//VECTOR
		glm::vec3 output = ReturnS() * rhs.ReturnV() + rhs.ReturnS() * ReturnV() + glm::cross(ReturnV(), rhs.ReturnV());
		mQuaternion.x = output.x;
		mQuaternion.y = output.y;
		mQuaternion.z = output.z;

		//SCALAR
		mQuaternion.w = ReturnS() * rhs.ReturnS() - glm::dot(ReturnV(), rhs.ReturnV());

		return *this;
	}

	Quaternion& Quaternion::operator=(Quaternion const& rhs)
	{
		mQuaternion = rhs.mQuaternion; return *this;
	}

	glm::mat4 Quaternion::ToMatrix()
	{
		glm::mat4 result = glm::mat4(1.0f);
		result[0][0] = 1 - 2 * (mQuaternion.y * mQuaternion.y + mQuaternion.z * mQuaternion.z);
		result[0][1] = 2 * (mQuaternion.x * mQuaternion.y - mQuaternion.w * mQuaternion.z);
		result[0][2] = 2 * (mQuaternion.x * mQuaternion.z + mQuaternion.w * mQuaternion.y);

		result[1][0] = 2 * (mQuaternion.x * mQuaternion.y + mQuaternion.w * mQuaternion.z);
		result[1][1] = 1 - 2 * (mQuaternion.x * mQuaternion.x + mQuaternion.z * mQuaternion.z);
		result[1][2] = 2 * (mQuaternion.y * mQuaternion.z - mQuaternion.w * mQuaternion.x);

		result[2][0] = 2 * (mQuaternion.x * mQuaternion.z - mQuaternion.w * mQuaternion.y);
		result[2][1] = 2 * (mQuaternion.y * mQuaternion.z + mQuaternion.w * mQuaternion.x);
		result[2][2] = 1 - 2 * (mQuaternion.y * mQuaternion.y + mQuaternion.x * mQuaternion.x);

		return glm::transpose(result);
	}

	glm::vec3 Quaternion::Rotate(glm::vec3 x)
	{
		return glm::vec3((ReturnS() * ReturnS() - glm::dot(ReturnV(), ReturnV())) * x
			+ 2 * (glm::dot(ReturnV(), x)) * ReturnV() + 2 * ReturnS() *
			(glm::cross(ReturnV(), x)));
	}

	aiMatrix4x4 Quaternion::ToAiRotationMatrix()
	{
		aiMatrix4x4 matrix;
		float xy = mQuaternion.x * mQuaternion.y;
		float xz = mQuaternion.x * mQuaternion.z;
		float xw = mQuaternion.x * mQuaternion.w;
		float yz = mQuaternion.y * mQuaternion.z;
		float yw = mQuaternion.y * mQuaternion.w;
		float zw = mQuaternion.z * mQuaternion.w;
		float xSquared = mQuaternion.x * mQuaternion.x;
		float ySquared = mQuaternion.y * mQuaternion.y;
		float zSquared = mQuaternion.z * mQuaternion.z;
		matrix.a1 = 1 - 2 * (ySquared + zSquared);
		matrix.a2 = 2 * (xy - zw);
		matrix.a3 = 2 * (xz + yw);
		matrix.a4 = 0;
		matrix.b1 = 2 * (xy + zw);
		matrix.b2 = 1 - 2 * (xSquared + zSquared);
		matrix.b3 = 2 * (yz - xw);
		matrix.b4 = 0;
		matrix.c1 = 2 * (xz - yw);
		matrix.c2 = 2 * (yz + xw);
		matrix.c3 = 1 - 2 * (xSquared + ySquared);
		matrix.c4 = 0;
		matrix.d1 = 0;
		matrix.d2 = 0;
		matrix.d3 = 0;
		matrix.d4 = 1;

		return matrix;
	}

	void Quaternion::Normalize()
	{
		glm::normalize(mQuaternion);
	}

	float Quaternion::operator[](unsigned int i)
	{
		return mQuaternion[i];
	}

	float Quaternion::Dot(Quaternion const& rhs)
	{
		return glm::dot(rhs.mQuaternion, mQuaternion);
	}

	glm::vec3 Quaternion::ReturnV() const
	{
		return glm::vec3(mQuaternion.x, mQuaternion.y, mQuaternion.z);
	}

	float Quaternion::ReturnS() const
	{
		return mQuaternion.w;
	}

	Quaternion Quaternion::Conjugate()
	{
		return Quaternion(-mQuaternion.x, -mQuaternion.y, -mQuaternion.z, mQuaternion.w);
	}

	Quaternion Quaternion::Inverse()
	{
		return Conjugate() / (glm::length(mQuaternion) * glm::length(mQuaternion));
	}

	Quaternion Quaternion::Slerp(Quaternion& A, Quaternion& B, float blend)
	{
		Quaternion end = B;
		float q0_, q1_, alpha_, sin_alpha;
		float cos = glm::dot(A.mQuaternion, B.mQuaternion);

		if (cos < 0.0f) {
			cos = -cos;
			end.mQuaternion = -end.mQuaternion;
		}

		if ((1.0f - cos) > 0.0001f) {
			alpha_ = std::acos(cos);
			sin_alpha = std::sin(alpha_);
			q0_ = std::sin((1.0f - blend) * alpha_) / sin_alpha;
			q1_ = std::sin(blend * alpha_) / sin_alpha;
		}
		else {
			q0_ = 1.0f - blend;
			q1_ = blend;
		}

		Quaternion finalResult;
		finalResult.mQuaternion = q0_ * A.mQuaternion + q1_ * end.mQuaternion;
		return finalResult;
	}

	Quaternion Quaternion::Nlerp(Quaternion a, Quaternion b, float blend)
	{
		a.Normalize();
		b.Normalize();

		Quaternion result;
		float dot_product = a.mQuaternion.x * b.mQuaternion.x + a.mQuaternion.y * b.mQuaternion.y + a.mQuaternion.z * b.mQuaternion.z + a.mQuaternion.w * b.mQuaternion.w;
		float one_minus_blend = 1.0f - blend;

		if (dot_product < 0.0f)
		{
			result.mQuaternion.x = a.mQuaternion.x * one_minus_blend + blend * -b.mQuaternion.x;
			result.mQuaternion.y = a.mQuaternion.y * one_minus_blend + blend * -b.mQuaternion.y;
			result.mQuaternion.z = a.mQuaternion.z * one_minus_blend + blend * -b.mQuaternion.z;
			result.mQuaternion.w = a.mQuaternion.w * one_minus_blend + blend * -b.mQuaternion.w;
		}
		else
		{
			result.mQuaternion.x = a.mQuaternion.x * one_minus_blend + blend * b.mQuaternion.x;
			result.mQuaternion.y = a.mQuaternion.y * one_minus_blend + blend * b.mQuaternion.y;
			result.mQuaternion.z = a.mQuaternion.z * one_minus_blend + blend * b.mQuaternion.z;
			result.mQuaternion.w = a.mQuaternion.w * one_minus_blend + blend * b.mQuaternion.w;
		}

		result.Normalize();
		return result;
	}

	Quaternion::~Quaternion()
	{
	}

	Quaternion& operator*(float const data, Quaternion& rhs)
	{
		return rhs * data;
	}
}