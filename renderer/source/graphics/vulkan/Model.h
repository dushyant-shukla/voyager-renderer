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

#include "VulkanTexture.h"
#include "Buffer.h"
#include "DescriptorSets.h"
#include "CCDSolver.h"

const unsigned int MAX_BONES = 100;
const unsigned int MAX_BONES_PER_VERTX = 4;
const unsigned int MAX_SAMPLER_DESCRIPTOR_COUNT = 150;

const std::string MODEL_PATH = "..\\..\\assets\\models\\";
const std::string TEXTURE_PATH = "..\\..\\assets\\textures\\";

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

	struct MeshVertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::ivec4 boneIds;				// each vertex is influenced by 4 bones at max
		glm::vec4 boneWeights;

		static VkVertexInputBindingDescription GetVertexInputBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(MeshVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> mInputAttributeDescriptions(7);

			// TODO: in what scenario binding here will be other than '0'
			mInputAttributeDescriptions[0].binding = 0;
			mInputAttributeDescriptions[0].location = 0;
			mInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			mInputAttributeDescriptions[0].offset = offsetof(MeshVertex, position);

			mInputAttributeDescriptions[1].binding = 0;
			mInputAttributeDescriptions[1].location = 1;
			mInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			mInputAttributeDescriptions[1].offset = offsetof(MeshVertex, color);

			mInputAttributeDescriptions[2].binding = 0;
			mInputAttributeDescriptions[2].location = 2;
			mInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			mInputAttributeDescriptions[2].offset = offsetof(MeshVertex, uv);

			mInputAttributeDescriptions[3].binding = 0;
			mInputAttributeDescriptions[3].location = 3;
			mInputAttributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			mInputAttributeDescriptions[3].offset = offsetof(MeshVertex, normal);

			mInputAttributeDescriptions[4].binding = 0;
			mInputAttributeDescriptions[4].location = 4;
			mInputAttributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
			mInputAttributeDescriptions[4].offset = offsetof(MeshVertex, tangent);

			mInputAttributeDescriptions[5].binding = 0;
			mInputAttributeDescriptions[5].location = 5;
			mInputAttributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SINT;
			mInputAttributeDescriptions[5].offset = offsetof(MeshVertex, boneIds);

			mInputAttributeDescriptions[6].binding = 0;
			mInputAttributeDescriptions[6].location = 6;
			mInputAttributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			mInputAttributeDescriptions[6].offset = offsetof(MeshVertex, boneWeights);

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

	struct Texture
	{
		enum class Type
		{
			DIFFUSE = 1,
			SPECULAR,
			AMBIENT,
			EMISSIVE,
			HEIGHT,
			NORMALS,
			SHININESS,
			OPACITY,
			DISPLACEMENT,
			LIGHTMAP,
			REFLECTION,

			// PBR
			BASE_COLOR,
			NORMAL_CAMERA,
			EMISSION_COLOR,
			METALNESS,
			DIFFUSE_ROUGHNESS,
			AMBIENT_OCCLUSION,
		};

		Type type;
		std::string path;
		vr::VulkanTexture* texture;

		Texture();
		Texture(Type _type, std::string _path);
		~Texture();
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

	struct Mesh
	{
		std::vector<MeshVertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture*> textures;

		/*
			Each mesh must own its descriptor set to send its unique data(mostly textures for now) to shader
		*/
		vr::DescriptorSets mDescriptorSets;

		struct
		{
			vr::Buffer<MeshVertex> vertex;
			vr::Buffer<unsigned int> index;
		} buffers;

		Mesh();
		Mesh(std::vector<MeshVertex> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures);
		~Mesh();

		/*
			record draw commands in the command buffer
		*/
		void Draw(const VkCommandBuffer& cmdBuffer);
	};

	struct Model
	{
		~Model();

		std::string mScreenName;

		struct
		{
			glm::vec3 position;
			glm::vec3 scale;
			glm::vec3 rotation;
		} mTransform;

		struct
		{
			glm::vec3 position;
			glm::vec3 scale;
			glm::vec3 rotation;
		} mAnimationTransform;

		struct
		{
			bool texturesAvailable = 1;
			glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
		} modelSettings;

		const aiScene* mScene = nullptr;
		Animation* mAnimation = nullptr;
		bool isAnimationAvailable = false;

		// these things are actually drawn
		std::vector<Mesh*> meshes;
		std::vector<glm::vec4> jointPositions;
		std::vector<glm::vec4> linePositions;

		void LoadFromFile(std::string filename, std::string screename);
		void QueryAnimationData();
		void QueryMeshData(const unsigned int& meshIndex, const aiMesh* mesh);
		void QueryMeshMaterial(const unsigned int& meshIndex, const aiMesh* mesh);

		static glm::mat4 ModelMatForLineBWTwoPoints(glm::vec3 A, glm::vec3 B);

	private:

		// utility constant
		static inline const int DEFAUTL_FLAGS = aiProcess_Triangulate | /*aiProcess_GenNormals | aiProcess_GenSmoothNormals | */aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded | aiProcess_GlobalScale;
		static inline const aiVector3D Zero3D = aiVector3D(0.0f, 0.0f, 0.0f);
	};
}
