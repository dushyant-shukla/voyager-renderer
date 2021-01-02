#pragma once

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

#include "Texture.h"
#include "graphics/vulkan/Buffer.h"
#include "graphics/vulkan/DescriptorSets.h"

namespace vrassimp
{
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
}
