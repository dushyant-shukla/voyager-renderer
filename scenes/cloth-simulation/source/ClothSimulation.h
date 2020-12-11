#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "main.h"
#include "Application.h"
#include "graphics/vulkan/DescriptorPool.h"
#include "graphics/vulkan/DescriptorSetLayout.h"
#include "graphics/vulkan/Pipeline.h"
#include "graphics/vulkan/PipelineLayout.h"
#include "graphics/vulkan/TextureSampler.h"
#include "graphics/vulkan/Model.h"

#include "Cloth.h"

namespace vr
{
	class ClothSimulation : public Application
	{
	public:

		ClothSimulation(std::string name);
		~ClothSimulation();

		virtual void InitializeScene() override;
		virtual void CleanupScene() override;
		virtual void Draw(const double& frametime) override;
		virtual VkPhysicalDeviceFeatures CheckRequiredFeatures() override;
		virtual void OnUpdateUIOverlay(UiOverlay* overlay) override;

	private:

		// setup methods
		void SetupDescriptorSet();
		void SetupPipeline();
		void SetupUbo();
		void SetupTextureSamplers();

		// loading assets
		void LoadAssets();

		// update methods
		void UpdateViewBuffer(unsigned int imageIndex);
		void UpdateModelData(vrassimp::Model* model, const double& frametime);
		void UpdateLightBuffer(unsigned int imageIndex);

		// draw commands
		void RecordCommands(unsigned int imageIndex, const double& frametime);

		//void CreateCloth();
		void UpdateCloth(unsigned int imageIndex);
		void DrawCloth(unsigned int imageIndex);

	private:

		struct
		{
			PipelineLayout mesh;
			PipelineLayout cloth;
		} mPipelineLayouts;

		struct
		{
			Pipeline mesh;
			Pipeline cloth;
		} mPipelines;

		struct
		{
			DescriptorSets mesh;
			DescriptorSets cloth;
		} mDescriptorSets;

		struct
		{
			DescriptorSetLayout mesh;
			DescriptorSetLayout texture;
			DescriptorSetLayout cloth;
		} mDescriptorSetLayouts;

		struct
		{
			DescriptorPool mesh;
			DescriptorPool texture;
			DescriptorPool cloth;
		} mDescriptorPools;

		struct
		{
			alignas(16) glm::mat4 projection;
			alignas(16) glm::mat4 view;
		} viewUBO;
		std::vector<VkBuffer> viewUboBuffers;
		std::vector<VkDeviceMemory> viewUboMemory;

		struct
		{
			alignas(16) glm::vec3 position;
			alignas(16) glm::vec3 color;
			alignas(4) float constant;
			alignas(4) float linear;
			alignas(4) float quadratic;
		} lightUBO;
		std::vector<VkBuffer> lightUboBuffers;
		std::vector<VkDeviceMemory> lightUboMemory;

		struct
		{
			alignas(16) glm::mat4 model;
		} modelData;

		struct
		{
			TextureSampler diffuse;
			TextureSampler normal;
		} mSamplers;

		vrassimp::Model* ball;
		vrassimp::Model* phantom;
		std::vector<vrassimp::Model*> mModels;

		float ballRadius = 0.5f;

		int mCurrentFrame = 0;

		const int MAX_MESH_COUNT = 100;

		struct ClothVertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;

			ClothVertex() :position(0.0f), normal(0.0f), uv(0.0)
			{}

			ClothVertex(glm::vec3 p, glm::vec3 n, glm::vec2 _uv) :position(p), normal(n), uv(_uv)
			{}

			ClothVertex(float px, float py, float pz, float nx, float ny, float nz, float s, float t)
				:position(px, py, pz), normal(nx, ny, nz), uv(s, t)
			{}

			static VkVertexInputBindingDescription GetVertexInputBindingDescription()
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(ClothVertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				return bindingDescription;
			}

			static std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributeDescriptions()
			{
				std::vector<VkVertexInputAttributeDescription> mInputAttributeDescriptions(3);

				mInputAttributeDescriptions[0].binding = 0;
				mInputAttributeDescriptions[0].location = 0;
				mInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				mInputAttributeDescriptions[0].offset = offsetof(ClothVertex, position);

				mInputAttributeDescriptions[1].binding = 0;
				mInputAttributeDescriptions[1].location = 1;
				mInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				mInputAttributeDescriptions[1].offset = offsetof(ClothVertex, normal);

				mInputAttributeDescriptions[2].binding = 0;
				mInputAttributeDescriptions[2].location = 2;
				mInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
				mInputAttributeDescriptions[2].offset = offsetof(ClothVertex, uv);

				return mInputAttributeDescriptions;
			}
		};

		struct
		{
			glm::vec3 position;
			glm::vec3 rotation;
			glm::vec3 scale;
		} clothModel;

		struct
		{
			int wind = 1;
			glm::vec3 windForce = glm::vec3(0.0f, 0.0f, -5.0f);
			glm::vec3 force = glm::vec3(0.0f, -0.02f, 0.002f);
			float radiusAdjustment = 1.0f;

			// pinned particles status : 1 for pinned, 0 for unpinned
			int upperLeft = 1;
			int upperRight = 1;
			int bottomLeft = 1;
			int bottomRight = 1;
		} mSettings;

		Cloth mCloth;
		Buffer<ClothVertex> mClothBuffer;
	};
}