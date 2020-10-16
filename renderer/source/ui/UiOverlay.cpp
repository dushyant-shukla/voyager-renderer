#include "UiOverlay.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <imgui.h>
#include <glm/glm.hpp>

#include "graphics/vulkan/utility/ImageUtility.h"
#include "graphics/vulkan/utility/RendererCoreUtility.h"
#include "RendererState.h"

namespace vr
{
	UiOverlay::UiOverlay()
	{
		/*
			Initialize ImGUI
		*/
		ImGui::CreateContext();

		glm::vec3 color = glm::vec3(0.03f, 0.3f, 1.0f);
		{
			//glm::vec3 color = glm::vec3(0.118f, 0.565f, 1.000f);
			//glm::vec3 color = glm::vec3(0.001f, 0.1f, 1.000f);
			//glm::vec4 color = glm::vec4(0.002f, 0.1617f, 0.30845f, 0.52736f);
			//glm::vec3 color = glm::vec3(0.275f, 0.510f, 0.706f);
			//glm::vec3 color = glm::vec3(0.098f, 0.098f, 0.439f);
			//glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f);
			//glm::vec3 color = glm::vec3(0.255f, 0.412f, 0.882f);
		}

		// configuring color scheme
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.97f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(color.x, color.y, color.z, 1.0);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(color.x, color.y, color.z, 1.0);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(color.x, color.y, color.z, 0.7f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(color.x, color.y, color.z, 0.4f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(color.x, color.y, color.z, 0.4f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(color.x, color.y, color.z, 0.4f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(color.x, color.y, color.z, 0.8f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(color.x, color.y, color.z, 0.4f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(color.x, color.y, color.z, 0.8f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
		style.Colors[ImGuiCol_Button] = ImVec4(color.x, color.y, color.z, 0.4f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(color.x, color.y, color.z, 0.6f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(color.x, color.y, color.z, 0.8f);
		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = mUiState.scale;
	}

	UiOverlay::~UiOverlay()
	{
	}

	void UiOverlay::PreparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass, const VkExtent2D extent)
	{
		mPipelineLayouts.pipelineLayout
			.AddDescriptorSetLayout(mDescriptors.layout.mLayout)
			.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mPushconstantBlock))
			.Configure();

		// setup pipeline
		mPipelines.pipeline // TODO setup shaders
			.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_PATH + "uioverlay.vert.spv")
			.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_PATH + "uioverlay.frag.spv")
			.ConfigureInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0, nullptr)
			.AddVertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX)
			.AddVertexInputAttributeDescription(0, VK_FORMAT_R32G32_SFLOAT, 0, offsetof(ImDrawVert, pos))
			.AddVertexInputAttributeDescription(0, VK_FORMAT_R32G32_SFLOAT, 1, offsetof(ImDrawVert, uv))
			.AddVertexInputAttributeDescription(0, VK_FORMAT_R8G8B8A8_UNORM, 2, offsetof(ImDrawVert, col))
			.ConfigureViewport(1, 1, extent, 0)
			.ConfigureRasterizer(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f, 0, nullptr)
			.ConfigureMultiSampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, 0, VK_FALSE, VK_FALSE, nullptr)
			.ConfigureDepthTesting(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS)
			.AddColorBlendAttachmentState(VK_TRUE,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_FACTOR_ZERO,
				VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
			.ConfigureColorBlendState(nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 0.0f, 0.0f, 0.0f, 0.0f)
			.Configure(mPipelineLayouts.pipelineLayout.mLayout, renderPass, 0, 0);
	}

	void UiOverlay::PrepareResources()
	{
		ImGuiIO& io = ImGui::GetIO();

		// font texture
		unsigned char* fontData;
		int textureWidth, textureHeight;

		const std::string filename = FONT_PATH + "Sansation-Bold.ttf";
		io.Fonts->AddFontFromFileTTF(filename.c_str(), 16.0f);
		io.Fonts->GetTexDataAsRGBA32(&fontData, &textureWidth, &textureHeight);

		VkDeviceSize uploadSize = textureWidth * textureHeight * 4 * sizeof(char);

		mFont.sampler
			.MagFilter(VK_FILTER_LINEAR)
			.MinFilter(VK_FILTER_LINEAR)
			.MipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			.AddressModeU(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
			.AddressModeV(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
			.AddressModeW(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
			.BorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
			.UnnormalizedCoordinates(VK_FALSE)
			.MipLodBias(0.0f)
			.MinLod(0.0f)
			.MaxLod(0.0f)
			.EnableAnistropy(VK_TRUE)
			.MaxAnistropy(16)
			.EnableCompare(VK_FALSE)
			.CompareOp(VK_COMPARE_OP_ALWAYS)
			.Configure();

		mFont.texture.LoadWithData(fontData, uploadSize, textureWidth, textureHeight, mFont.sampler.GetVulkanSampler());

		mDescriptors.pool
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.Create(0, 2, nullptr);

		mDescriptors.layout
			.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr)
			.Create(0, nullptr);

		mDescriptors.sets.Setup(mDescriptors.layout.mLayout,
			mDescriptors.pool.mPool,
			1);

		VkWriteDescriptorSet writeBufferInfo = {};
		writeBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeBufferInfo.dstSet = mDescriptors.sets[0];
		writeBufferInfo.dstBinding = 0;
		writeBufferInfo.dstArrayElement = 0;
		writeBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeBufferInfo.descriptorCount = 1;
		writeBufferInfo.pBufferInfo = nullptr;
		writeBufferInfo.pImageInfo = &mFont.texture.mImageInfo;
		writeBufferInfo.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(LOGICAL_DEVICE, 1, &writeBufferInfo, 0, nullptr);
	}

	bool UiOverlay::Update()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		bool updateCmdBuffers = false;

		if (!imDrawData)
		{
			return false;
		}

		// Note: Alignment is done inside buffer creation
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		// Update buffers only if vertex or index count has been changed compared to current buffer size
		if ((vertexBufferSize == 0) || (indexBufferSize == 0))
		{
			return false;
		}

		// Vertex buffer
		if ((buffers.vertex.mBuffer == VK_NULL_HANDLE) || (buffers.vertexCount != imDrawData->TotalVtxCount))
		{
			buffers.vertex.Unmap();
			vkQueueWaitIdle(GRAPHICS_QUEUE);
			buffers.vertex.Destroy();
			MemoryUtility::CreateBuffer(vertexBufferSize,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				ALLOCATION_CALLBACK,
				&buffers.vertex.mBuffer,
				&buffers.vertex.mMemory);

			buffers.vertex.SetupDescriptor();
			buffers.vertexCount = imDrawData->TotalVtxCount;
			buffers.vertex.Unmap();
			buffers.vertex.Map();
			updateCmdBuffers = true;
		}

		// Index buffer
		VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
		if ((buffers.index.mBuffer == VK_NULL_HANDLE) || (buffers.indexCount < imDrawData->TotalIdxCount))
		{
			buffers.index.Unmap();
			vkQueueWaitIdle(GRAPHICS_QUEUE);
			buffers.index.Destroy();
			MemoryUtility::CreateBuffer(indexBufferSize,
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				ALLOCATION_CALLBACK,
				&buffers.index.mBuffer,
				&buffers.index.mMemory);
			buffers.index.SetupDescriptor();
			buffers.indexCount = imDrawData->TotalIdxCount;
			buffers.index.Map();
			updateCmdBuffers = true;
		}

		//Upload data
		ImDrawVert* vtxDst = (ImDrawVert*)buffers.vertex.mapped;
		ImDrawIdx* idxDst = (ImDrawIdx*)buffers.index.mapped;

		for (int n = 0; n < imDrawData->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[n];
			memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			idxDst += cmd_list->IdxBuffer.Size;
		}

		//Flush to make writes visible to GPU
		buffers.vertex.Flush();
		buffers.index.Flush();

		return updateCmdBuffers;
	}

	void UiOverlay::Draw(const VkCommandBuffer commandBuffer)
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.pipeline.GetVulkanPipeline());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayouts.pipelineLayout.mLayout, 0, 1, &mDescriptors.sets[0], 0, NULL);

		mPushconstantBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		mPushconstantBlock.translate = glm::vec2(-1.0f);
		vkCmdPushConstants(commandBuffer, mPipelineLayouts.pipelineLayout.mLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mPushconstantBlock), &mPushconstantBlock);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers.vertex.mBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, buffers.index.mBuffer, 0, VK_INDEX_TYPE_UINT16);

		for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
				vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}
	}

	void UiOverlay::Resize(unsigned int width, unsigned int height)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)(width), (float)(height));
	}

	void UiOverlay::FreeResources()
	{
		ImGui::DestroyContext();
	}

	bool UiOverlay::Header(const char* caption)
	{
		return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
	}

	bool UiOverlay::CheckBox(const char* caption, bool* value)
	{
		bool res = ImGui::Checkbox(caption, value);
		if (res)
		{
			mUiState.updated = true;
		};
		return res;
	}

	bool UiOverlay::CheckBox(const char* caption, int32_t* value)
	{
		bool val = (*value == 1);
		bool res = ImGui::Checkbox(caption, &val);
		*value = val;
		if (res)
		{
			mUiState.updated = true;
		};
		return res;
	}

	bool UiOverlay::InputFloat(const char* caption, float* value, float step, unsigned int precision)
	{
		bool res = ImGui::InputFloat(caption, value, step, step * 10.0f, precision);
		if (res)
		{
			mUiState.updated = true;
		};
		return res;
	}

	bool UiOverlay::SliderFloat(const char* caption, float* value, float min, float max)
	{
		bool res = ImGui::SliderFloat(caption, value, min, max);
		if (res)
		{
			mUiState.updated = true;
		};
		return res;
	}

	bool UiOverlay::SliderInt(const char* caption, int32_t* value, int32_t min, int32_t max)
	{
		bool res = ImGui::SliderInt(caption, value, min, max);
		if (res)
		{
			mUiState.updated = true;
		};
		return res;
	}

	bool UiOverlay::ComboBox(const char* caption, int32_t* itemindex, std::vector<std::string> items)
	{
		if (items.empty())
		{
			return false;
		}
		std::vector<const char*> charitems;
		charitems.reserve(items.size());
		for (size_t i = 0; i < items.size(); i++)
		{
			charitems.push_back(items[i].c_str());
		}
		uint32_t itemCount = static_cast<uint32_t>(charitems.size());
		bool     res = ImGui::Combo(caption, itemindex, &charitems[0], itemCount, itemCount);
		if (res)
		{
			mUiState.updated = true;
		};
		return res;
	}

	bool UiOverlay::Button(const char* caption)
	{
		bool res = ImGui::Button(caption);
		if (res)
		{
			mUiState.updated = true;
		};
		return res;
	}

	void UiOverlay::Text(const char* formatstr, ...)
	{
		va_list args;
		va_start(args, formatstr);
		ImGui::TextV(formatstr, args);
		va_end(args);
	}
}