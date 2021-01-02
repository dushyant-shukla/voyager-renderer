#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <array>
#include <map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "graphics/model/Texture.h"
#include "graphics/vulkan/VulkanTexture.h"
#include "graphics/vulkan/Buffer.h"
#include "graphics/vulkan/DescriptorSets.h"
#include "CCDSolver.h"

#include "graphics/model/Mesh.h"

const unsigned int MAX_BONES = 100;
const unsigned int MAX_BONES_PER_VERTX = 4;
const unsigned int MAX_SAMPLER_DESCRIPTOR_COUNT = 150;

namespace vrassimp
{
	struct BoneVertex
	{
		glm::vec4 position;

		static VkVertexInputBindingDescription GetVertexInputBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(BoneVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> mInputAttributeDescriptions(1);

			// TODO: in what scenario binding here will be other than '0'
			mInputAttributeDescriptions[0].binding = 0;
			mInputAttributeDescriptions[0].location = 0;
			mInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			mInputAttributeDescriptions[0].offset = offsetof(BoneVertex, position);

			return mInputAttributeDescriptions;
		}
	};

	struct JointVertex
	{
		glm::vec4 position;

		static VkVertexInputBindingDescription GetVertexInputBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(JointVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> mInputAttributeDescriptions(1);

			// TODO: in what scenario binding here will be other than '0'
			mInputAttributeDescriptions[0].binding = 0;
			mInputAttributeDescriptions[0].location = 0;
			mInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			mInputAttributeDescriptions[0].offset = offsetof(JointVertex, position);

			return mInputAttributeDescriptions;
		}
	};

	/*
		Per vertex bone ids and weights
	*/
	struct VertexBoneData
	{
		std::array<unsigned int, MAX_BONES_PER_VERTX> ids;
		std::array<float, MAX_BONES_PER_VERTX> weights;

		void AddBoneData(int id, float weight);
	};

	/*
		Stores information on a single bone
	*/
	struct BoneMatrix
	{
		/*
			Vertices are stored in the usual mesh local space (this enables us to load models without animation support correctly).
			But bone transformations work in bone spaces, and every bone has its own space. So the job of the offset matric is to move
			the vertex position from local space of the mesh into bone space of that particular bone.
		*/
		aiMatrix4x4 offset;
		aiMatrix4x4 finalWorldTransform;
		aiMatrix4x4 finalBoneTransform;
	};

	struct BoneLine
	{
		aiMatrix4x4 parentBone;
		aiMatrix4x4 childBone;

		BoneLine(aiMatrix4x4 pBone, aiMatrix4x4 cBone) :
			parentBone(pBone), childBone(cBone) {}
	};

	struct Animation
	{
		float mTicksPerSecond = 0.0f;

		int mBoneCount = 0;

		/*
			simulates a 2D array with rows = meshes and columns = vertices
		*	[mesh, vertex] = {bone ids (4), bone weights (4)}
		*/
		std::vector<std::vector<VertexBoneData>> mBoneIdsAndWeights;

		/*
			bone name -> bone index
		*/
		std::map<std::string, int> mBoneMapping;
		std::vector<BoneMatrix> mBoneMatrices;
		aiMatrix4x4 mGlobalInverseTransform;

		std::vector<BoneLine> boneEndpointPositions;

		std::vector<std::string> mIKBoneNames;
		vr::CCDSolver mCCDSolver;

		const aiScene* mScene;

		struct AnimationTrack
		{
			double duration;
			std::string name;
			double ticksPerSecond;
			double start;
			double end;

			AnimationTrack(double d, std::string n, double t, double s, double e) : duration(d), name(n), ticksPerSecond(t), start(s), end(e)
			{}
		};
		std::vector<AnimationTrack> availableTracks;
		float timer = 0.0f;

		struct AnimationSettings
		{
			float speed = 0.75f;
			float currentSpeed = 0.75f;
			int enableAnimation = 0; // load model in bind pose
			std::string tracks = "";
			int currentTrackIndex = 0; // first track
			float pathTime = 0.0; // for implementing a walk-cycle along a curve
			int enableIk = 0;
			int showJoints = 0;
			int showLines = 0;
			int showMesh = 1;
			float uvOffsetScale = 0.0;
		} settings;

		void ProcessMesh(int meshIndex, aiMesh* mesh);
		void Animate(double seconds, std::vector<aiMatrix4x4>& transforms, std::vector<aiMatrix4x4>& boneTransforms);
		void ReadNodeHierarchy(float parentAnimationTime, const aiNode* parentNode, const aiMatrix4x4 parentTransform);
		const aiNodeAnim* FindNodeAnim(const aiAnimation* parentAnimation, const std::string parentNodeName);

		aiMatrix4x4 Animation::InterpolateScale(float time, const aiNodeAnim* pNodeAnim);
		int FindScaling(float parentAnimationTime, const aiNodeAnim* parentNodeAnimation);

		aiMatrix4x4 Animation::InterpolateRotation(float time, const aiNodeAnim* pNodeAnim);
		int FindRotation(float parentAnimationTime, const aiNodeAnim* parentNodeAnimation);

		aiMatrix4x4 InterpolateTranslation(float time, const aiNodeAnim* pNodeAnim);
		int FindPosition(float parentAnimationTime, const aiNodeAnim* parentNodeAnimation);

		glm::mat4 aiToGlm(aiMatrix4x4 ai_matr);

		void InitializeIKData();
		void Animation::ReadIKBoneHierarchy(float animationTime, const aiNode* parentNode, const aiMatrix4x4 parentTransform);
	};
}
