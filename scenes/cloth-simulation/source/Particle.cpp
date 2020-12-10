//#include "Particle.h"
//
//vr::Particle::Particle() : mPosition(0.0), mUV(0.0)
//{
//}
//
//vr::Particle::Particle(glm::vec3 position, glm::vec2 uv)
//	: mPosition(position), mOldPosition(position), mAcceleration(glm::vec3(0.0f)), mMass(1), mMovable(true), mAccumulatedNormal(glm::vec3(0.0f)), mUV(uv)
//{
//}
//
//vr::Particle::~Particle()
//{
//}
//
//vr::Particle& vr::Particle::operator=(const Particle& other)
//{
//	mMovable = other.mMovable;
//	mMass = other.mMass;
//	mPosition = other.mPosition;
//	mOldPosition = other.mOldPosition;
//	mAcceleration = other.mAcceleration;
//	mAccumulatedNormal = other.mAccumulatedNormal;
//	mUV = other.mUV;
//	return *this;
//}
//
//void vr::Particle::AddForce(glm::vec3 force)
//{
//	mAcceleration += force / mMass;
//}
//
//void vr::Particle::TimeStep()
//{
//	if (mMovable)
//	{
//		glm::vec3 temp(mPosition);
//		mPosition += (mPosition - mOldPosition) * (1.0f - DAMPING) + mAcceleration * TIME_STEP;
//		mOldPosition = temp;
//
//		// acceleration is reset since it has been translated
//		// into a change in position (and implicitly into velocity)
//		mAcceleration = glm::vec3(0.0f);
//	}
//}
//
//glm::vec3& vr::Particle::GetPosition()
//{
//	return mPosition;
//}
//
//glm::vec2& vr::Particle::GetUV()
//{
//	return mUV;
//}
//
//void vr::Particle::ResetAcceleration()
//{
//	mAcceleration = glm::vec3(0.0f);
//}
//
//void vr::Particle::OffsetPosition(const glm::vec3 offset)
//{
//	if (mMovable)
//	{
//		mPosition += offset;
//	}
//}
//
//void vr::Particle::MakeUnmovable()
//{
//	mMovable = false;
//}
//
//void vr::Particle::AddToNormal(const glm::vec3 normal)
//{
//	const glm::vec3 N = glm::normalize(normal);
//	mAccumulatedNormal += N;
//}
//
//glm::vec3& vr::Particle::GetNormal()
//{
//	return mAccumulatedNormal;
//}
//
//void vr::Particle::ResetNormal()
//{
//	mAccumulatedNormal = glm::vec3(0.0f);
//}