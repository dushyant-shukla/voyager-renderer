#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

namespace vr
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec3 normal;

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
			std::vector<VkVertexInputAttributeDescription> mInputAttributeDescriptions(4);

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

			return mInputAttributeDescriptions;
		}
	};
}
