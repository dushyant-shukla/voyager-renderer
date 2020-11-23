#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <map>
#include <string>

namespace vr
{
	struct DecomposedTransforms
	{
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;

		DecomposedTransforms(glm::vec3 t_scale, glm::quat t_rotation, glm::vec3 t_translation, glm::vec3 t_skew, glm::vec4 t_perspective)
			: scale(t_scale), rotation(t_rotation), translation(t_translation), skew(t_skew), perspective(t_perspective)
		{}
	};

	struct IKJoint
	{
		std::string name;
		int id;
		glm::mat4 localTransform;
		glm::mat4 globalTransform;
		glm::mat4 worldTransform;

		IKJoint() : name(""), id(0), localTransform(glm::mat4(1.0f)),
			globalTransform(glm::mat4(1.0f)), worldTransform(glm::mat4(1.0f))
		{}

		IKJoint(std::string _name) : name(_name), id(0), localTransform(glm::mat4(1.0f)),
			globalTransform(glm::mat4(1.0f)), worldTransform(glm::mat4(1.0f))
		{}
	};

	class CCDSolver
	{
	public:

		CCDSolver();
		~CCDSolver();

		void SetChainSize(unsigned int size);
		unsigned int GetChainSize();

		void SetStepCount(unsigned int count);
		unsigned int& GetStepCount();

		void SetThreshold(unsigned int threshold);
		float& GetThreshold();

		glm::mat4& operator[](unsigned int index);
		glm::mat4 GetGlobalTransformByBoneId(unsigned int index);

		bool Solve(const glm::vec3 target);

		void AddIkBone(std::string boneName);
		void UpdateIkMatrices(std::string bonename, unsigned int boneIndex, glm::mat4 localTransform, glm::mat4 globalTransform, glm::mat4 worldTransform);

	private:

		glm::mat4 GetGlobalTransform(unsigned int index);
		DecomposedTransforms Decompose(const glm::mat4& transforms);
		glm::quat FromTo(const glm::vec3& from, const glm::vec3& to);

	public:

		unsigned int mStepCount;
		float mThreshold;
		int mJointCount = 0;

		std::vector<IKJoint> mIkTransforms;

		/*
			first = bone id in skeleton
			second = index in mIkTransforms
		*/
		std::map<int, int> mSkeletonBoneIndexToIkBoneIndexMap;
	};
}
