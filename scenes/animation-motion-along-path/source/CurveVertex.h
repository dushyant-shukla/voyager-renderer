#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

namespace vr
{
	struct CurveVertex
	{
		glm::vec4 position;

		CurveVertex(float x, float y, float z)
		{
			position.x = x;
			position.y = y;
			position.z = z;
			position.w = 1.0f;
		}

		CurveVertex(glm::vec4 p)
		{
			position = p;
		}

		static VkVertexInputBindingDescription GetVertexInputBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(CurveVertex);
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
			mInputAttributeDescriptions[0].offset = offsetof(CurveVertex, position);

			return mInputAttributeDescriptions;
		}
	};
}
