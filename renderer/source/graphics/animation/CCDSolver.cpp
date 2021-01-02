#include <glm/gtx/matrix_decompose.hpp>

#include "CCDSolver.h"

//#define GLM_ENABLE_EXPERIMENTAL

vr::CCDSolver::CCDSolver() : mStepCount(15), mThreshold(0.00001f)
{
}

vr::CCDSolver::~CCDSolver()
{
}

void vr::CCDSolver::SetChainSize(unsigned int size)
{
	mIkTransforms.resize(size);
}

unsigned int vr::CCDSolver::GetChainSize()
{
	return mIkTransforms.size();
}

void vr::CCDSolver::SetStepCount(unsigned int count)
{
	mStepCount = count;
}

unsigned int& vr::CCDSolver::GetStepCount()
{
	return mStepCount;
}

void vr::CCDSolver::SetThreshold(unsigned int threshold)
{
	mThreshold = threshold;
}

float& vr::CCDSolver::GetThreshold()
{
	return mThreshold;
}

// TODO: rethink this
glm::mat4& vr::CCDSolver::operator[](unsigned int index)
{
	return mIkTransforms[index].localTransform;
}

glm::mat4 vr::CCDSolver::GetGlobalTransform(unsigned int index)
{
	//unsigned int size = mIkTransforms.size();
	//glm::mat4 world = mIkTransforms[index].localTransform;
	//for (int i = index - 1; i >= 0; --i)
	//{
	//	world = mIkTransforms[i].localTransform * world;
	//}
	//return world;

	unsigned int size = mIkTransforms.size();
	glm::mat4 world = mIkTransforms[index].globalTransform;
	for (int i = index - 1; i >= 0; --i)
	{
		world = mIkTransforms[i].globalTransform * world;
	}
	return world;
}

glm::mat4 vr::CCDSolver::GetGlobalTransformByBoneId(unsigned int index)
{
	if (mSkeletonBoneIndexToIkBoneIndexMap.find(index) != mSkeletonBoneIndexToIkBoneIndexMap.end())
	{
		unsigned int id = mSkeletonBoneIndexToIkBoneIndexMap[index];
		return GetGlobalTransform(id);
	}
	return glm::mat4(1.0f);
}

bool vr::CCDSolver::Solve(const glm::vec3 target)
{
	unsigned int size = mIkTransforms.size();
	if (size == 0)
	{
		return false;
	}

	unsigned int last = size - 1;
	float thresoldSq = mThreshold * mThreshold;

	for (int i = 0; i < mStepCount; ++i)
	{
		glm::vec3 effector = Decompose(GetGlobalTransform(last)).translation;
		glm::vec3 length = target - effector;
		float lengthValue = glm::length(length);
		if (lengthValue * lengthValue < thresoldSq)
		{
			return true;
		}

		for (int j = size - 2; j >= 0; --j) // size - 2 = last - 1
		{
			glm::mat4 world = GetGlobalTransform(j);
			DecomposedTransforms transforms = Decompose(world);
			glm::vec3 position = transforms.translation;
			glm::quat rotation = transforms.rotation;

			glm::vec3 toEffector = effector - position;
			glm::vec3 toTarget = target - position;
			float toTargetLength = glm::length(toTarget);
			glm::quat effectorToTarget;
			if (toTargetLength * toTargetLength > 0.00001f)
			{
				effectorToTarget = FromTo(toEffector, toTarget);
			}

			glm::quat worldRotation = rotation * effectorToTarget;
			//glm::quat localRotated = worldRotation * glm::inverse(rotation);
			glm::mat4 localRotation = glm::mat4_cast(worldRotation * glm::inverse(rotation));

			////mIKChain[j].rotation = localRotate * mIKChain[j].rotation;
			glm::mat4 newTransformation(1.0);
			newTransformation = glm::translate(newTransformation, position);
			newTransformation *= localRotation;
			newTransformation = glm::scale(newTransformation, transforms.scale);
			//mIkTransforms[j].localTransform = newTransformation;
			mIkTransforms[j].globalTransform = newTransformation;

			effector = Decompose(GetGlobalTransform(last)).translation;
			glm::vec3 targetToEffector = target - effector;
			float targetToEffectorLength = glm::length(targetToEffector);
			if (targetToEffectorLength * targetToEffectorLength < thresoldSq)
			{
				return true;
			}
		}
	}
	return false;
} // CCDSolver::Solve ends

vr::DecomposedTransforms vr::CCDSolver::Decompose(const glm::mat4& transform)
{
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(transform, scale, rotation, translation, skew, perspective); // rotation is conjugate
	rotation = glm::conjugate(rotation);
	return DecomposedTransforms(scale, rotation, translation, skew, perspective);
}

glm::quat vr::CCDSolver::FromTo(const glm::vec3& from, const glm::vec3& to)
{
	glm::vec3 f = glm::normalize(from);
	glm::vec3 t = glm::normalize(to);

	if (f == t)
	{
		return glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (f == t * -1.0f)
	{
		glm::vec3 ortho = glm::vec3(1.0, 0.0, 0.0);
		if (fabsf(f.y) < fabs(f.x))
		{
			ortho = glm::vec3(0.0, 1.0, 0.0);
		}
		if (fabsf(f.z) < fabsf(f.y) && fabsf(f.z) < fabsf(f.x))
		{
			ortho = glm::vec3(0.0, 0.0, 1.0);
		}

		glm::vec3 axis = glm::normalize(glm::cross(f, ortho));
		return glm::quat(axis.x, axis.y, axis.z, 0.0);
	}

	glm::vec3 half = glm::normalize(f + t);
	glm::vec3 axis = glm::cross(f, half);
	return glm::quat(axis.x, axis.y, axis.z, glm::dot(f, half));
}

void vr::CCDSolver::AddIkBone(std::string boneName)
{
	IKJoint newJoint(boneName);
	mIkTransforms.push_back(newJoint);
}

void vr::CCDSolver::UpdateIkMatrices(std::string bonename, unsigned int boneIndex, glm::mat4 localTransform, glm::mat4 globalTransform, glm::mat4 worldTransform)
{
	for (size_t i = 0; i < mIkTransforms.size(); ++i)
	{
		IKJoint joint = mIkTransforms[i];
		if (joint.name._Equal(bonename))
		{
			joint.localTransform = localTransform;
			joint.globalTransform = globalTransform;
			joint.worldTransform = worldTransform;
			joint.id = boneIndex;
			mIkTransforms[i] = joint;
			mSkeletonBoneIndexToIkBoneIndexMap[boneIndex] = i;
		}
	}
}