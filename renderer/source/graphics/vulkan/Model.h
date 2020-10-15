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

#include "Texture.h"
#include "maths/Quaternions.h"
#include "Buffer.h"
#include "DescriptorSets.h"

const unsigned int MAX_BONES = 100;
const unsigned int MAX_BONES_PER_VERTX = 4;
const unsigned int MAX_SAMPLER_DESCRIPTOR_COUNT = 150;

const std::string MODEL_PATH = "..\\..\\assets\\models\\";
const std::string TEXTURE_PATH = "..\\..\\assets\\textures\\";

namespace vrassimp
{
	struct Vertex
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
			bindingDescription.stride = sizeof(Vertex);
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
			mInputAttributeDescriptions[0].offset = offsetof(Vertex, position);

			mInputAttributeDescriptions[1].binding = 0;
			mInputAttributeDescriptions[1].location = 1;
			mInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			mInputAttributeDescriptions[1].offset = offsetof(Vertex, color);

			mInputAttributeDescriptions[2].binding = 0;
			mInputAttributeDescriptions[2].location = 2;
			mInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			mInputAttributeDescriptions[2].offset = offsetof(Vertex, uv);

			mInputAttributeDescriptions[3].binding = 0;
			mInputAttributeDescriptions[3].location = 3;
			mInputAttributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			mInputAttributeDescriptions[3].offset = offsetof(Vertex, normal);

			mInputAttributeDescriptions[4].binding = 0;
			mInputAttributeDescriptions[4].location = 4;
			mInputAttributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
			mInputAttributeDescriptions[4].offset = offsetof(Vertex, tangent);

			mInputAttributeDescriptions[5].binding = 0;
			mInputAttributeDescriptions[5].location = 5;
			mInputAttributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SINT;
			mInputAttributeDescriptions[5].offset = offsetof(Vertex, boneIds);

			mInputAttributeDescriptions[6].binding = 0;
			mInputAttributeDescriptions[6].location = 6;
			mInputAttributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			mInputAttributeDescriptions[6].offset = offsetof(Vertex, boneWeights);

			return mInputAttributeDescriptions;
		}
	};

	enum class VertexLayoutComponentType
	{
		VERTEX_COMPONENT_POSITION = 0x0,
		VERTEX_COMPONENT_NORMAL = 0x1,
		VERTEX_COMPONENT_COLOR = 0x2,
		VERTEX_COMPONENT_UV = 0x3,
		VERTEX_COMPONENT_TANGENT = 0x4,
		VERTEX_COMPONENT_BITANGENT = 0x5,
		VERTEX_COMPONENT_BONE_WEIGHT = 0x6,
		VERTEX_COMPONENT_BONE_IDS = 0x7,
		VERTEX_COMPONENT_DUMMY_FLOAT = 0x8,
		VERTEX_COMPONENT_DUMMY_VEC4 = 0x9
	};

	struct VertexLayout
	{
		std::vector<VertexLayoutComponentType> layoutComponents;

		VertexLayout(std::vector<VertexLayoutComponentType> components);

		unsigned int Stride();
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
		aiMatrix4x4 offset;
		aiMatrix4x4 finalWorldTransform;
		aiMatrix4x4 finalBoneTransform;
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
		vr::Texture* texture;

		Texture();
		Texture(Type _type, std::string _path);
		~Texture();
	};

	struct Animation
	{
		float mTicksPerSecond = 0.0f;
		aiMatrix4x4 mGlobalInverseTransform;

		int mBoneCount = 0;
		/*
			simulates a 2D array with rows = meshes and columns = vertices
		*	[mesh, vertex] = {bone ids (4), bone weights (4)}
		*/
		std::vector<std::vector<VertexBoneData>> mBones;

		/*
			bone name -> bone index
		*/
		std::map<std::string, int> mBoneMapping;
		std::vector<BoneMatrix> mBoneMatrices;
		std::vector<glm::vec3> mBonePosition;
		std::vector <std::vector<int>> tempCount; // TODO: what the purpose of this variable

		bool mEnableSlerp;

		const aiScene* mScene;
		aiAnimation* mActiveAnimation;

		struct AnimationTimer
		{
			double duration;
			std::string name;
			double ticksPerSecond;
			double start;
			double end;

			AnimationTimer(double d, std::string n, double t, double s, double e) : duration(d), name(n), ticksPerSecond(t), start(s), end(e)
			{}
		};
		std::vector<AnimationTimer> animationTimes;
		unsigned int currentIndex = 0;

		void SetAnimation(unsigned int animationIndex);
		//void ProcessNode(aiNode* node, const aiScene* scene);
		void ProcessMesh(int meshIndex, aiMesh* mesh, const aiScene* scene);
		void BoneTransform(double seconds, std::vector<aiMatrix4x4>& transforms);
		void ReadNodeHierarchy(float parentAnimationTime, const aiNode* parentNode, const aiMatrix4x4 parentTransform);
		const aiNodeAnim* FindNodeAnim(const aiAnimation* parentAnimation, const std::string parentNodeName);

		aiMatrix4x4 Animation::InterpolateScale(float time, const aiNodeAnim* pNodeAnim);
		int FindScaling(float parentAnimationTime, const aiNodeAnim* parentNodeAnimation);

		aiMatrix4x4 Animation::InterpolateRotation(float time, const aiNodeAnim* pNodeAnim);
		int FindRotation(float parentAnimationTime, const aiNodeAnim* parentNodeAnimation);

		aiMatrix4x4 InterpolateTranslation(float time, const aiNodeAnim* pNodeAnim);
		int FindPosition(float parentAnimationTime, const aiNodeAnim* parentNodeAnimation);

		Quaternions nlerp(Quaternions a, Quaternions b, float blend);
		glm::mat4 aiToGlm(aiMatrix4x4 ai_matr);
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture*> textures;

		/*
			Each mesh must own its descriptor set to send its unique data(mostly textures for now) to shader
		*/
		vr::DescriptorSets mDescriptorSets;

		struct
		{
			vr::Buffer<Vertex> vertex;
			vr::Buffer<unsigned int> index;
		} buffers;

		Mesh();
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures);
		~Mesh();

		/*
			record draw commands in the command buffer
		*/
		void Draw(const VkCommandBuffer& cmdBuffer);
	};

	struct Model
	{
		~Model();

		/*
			Used for parameterizing model loading
		*/
		struct ModelCreateInfo
		{
			glm::vec3 center;
			glm::vec3 scale;
			glm::vec2 uvScale;
			VkMemoryPropertyFlags memoryPropertyFlags = 0;

			ModelCreateInfo();
			ModelCreateInfo(glm::vec3 _center, glm::vec3 _scale, glm::vec2 _uvScale);
			ModelCreateInfo(float _center, float _scale, float _uvScale);
		};

		Animation* mAnimation = nullptr;
		bool isAnimationAvailable = false;

		std::vector<Mesh*> meshes;

		void LoadFromFile(std::string filename, ModelCreateInfo* createInfo);
		void QueryAnimationData(Assimp::Importer& Importer, const aiScene* scene);
		void QueryMeshData(const unsigned int& meshIndex, const aiMesh* mesh, const aiScene* scene);
		void QueryMeshMaterial(const unsigned int& meshIndex, const aiMesh* mesh, const aiScene* scene);

		void Draw();

	private:

		// utility constant
		static inline const int DEFAUTL_FLAGS = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded | aiProcess_GlobalScale;
		static inline const aiVector3D Zero3D = aiVector3D(0.0f, 0.0f, 0.0f);
	};
}
