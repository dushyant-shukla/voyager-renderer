#include "Mesh.h"

namespace vrassimp
{
	Mesh::Mesh() {}

	Mesh::Mesh(std::vector<MeshVertex> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures)
		: vertices(vertices), indices(indices), textures(textures)
	{
	}

	Mesh::~Mesh()
	{
		for (auto& texture : textures)
		{
			if (texture != nullptr)
			{
				delete texture;
			}
		}
	}

	void Mesh::Draw(const VkCommandBuffer& cmdBuffer)
	{
		VkBuffer vertexBuffers[] = { buffers.vertex.GetVulkanBuffer() };	// buffers to bind
		VkDeviceSize offsets[] = { 0 };										// offsets into buffers being bound
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);	// command to bind vertex buffers before drawing with them

																			// bind mesh index buffer
		vkCmdBindIndexBuffer(cmdBuffer, buffers.index.GetVulkanBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmdBuffer, indices.size(), 1, 0, 0, 0);
	}
}