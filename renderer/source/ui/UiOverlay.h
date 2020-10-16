#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/vulkan/Buffer.h"
#include "graphics/vulkan/Device.h"
#include "graphics/vulkan/Pipeline.h"
#include "graphics/vulkan/DescriptorPool.h"
#include "graphics/vulkan/DescriptorSetLayout.h"
#include "graphics/vulkan/PipelineLayout.h"
#include "graphics/vulkan/DescriptorSets.h"
#include "graphics/vulkan/Texture.h"
#include "graphics/vulkan/TextureSampler.h"

namespace vr
{
	const std::string FONT_PATH = "..\\..\\assets\\fonts\\";
	const std::string SHADER_PATH = "..\\..\\assets\\shaders\\ui\\";

	class UiOverlay
	{
	public:

		UiOverlay();
		~UiOverlay();

		void PreparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass, const VkExtent2D extent);
		void PrepareResources();

		bool Update();
		void Draw(const VkCommandBuffer commandBuffer);
		void Resize(unsigned int width, unsigned int height);

		void FreeResources();

		bool Header(const char* caption);
		bool CheckBox(const char* caption, bool* value);
		bool CheckBox(const char* caption, int32_t* value);
		bool InputFloat(const char* caption, float* value, float step, unsigned int precision);
		bool SliderFloat(const char* caption, float* value, float min, float max);
		bool SliderInt(const char* caption, int32_t* value, int32_t min, int32_t max);
		bool ComboBox(const char* caption, int32_t* itemindex, std::vector<std::string> items);
		bool Button(const char* caption);
		void Text(const char* formatstr, ...);

	public:

		Device* mDevice = { nullptr };

		struct
		{
			Buffer<float> vertex;
			Buffer<unsigned int> index;
			unsigned int vertexCount;
			unsigned int indexCount;
		} buffers;

		struct
		{
			Pipeline pipeline;
		} mPipelines;

		struct
		{
			PipelineLayout pipelineLayout;
		} mPipelineLayouts;

		struct
		{
			DescriptorPool pool;
			DescriptorSetLayout layout;
			DescriptorSets sets;
		} mDescriptors;

		struct
		{
			glm::vec2 scale;
			glm::vec2 translate;
		} mPushconstantBlock;

		struct
		{
			bool visible = true;
			bool updated = false;
			float scale = 1.0;
		} mUiState;

		struct
		{
			Texture texture;
			TextureSampler sampler;
		} mFont;

	private:

		const VkSampleCountFlagBits raterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		unsigned int subpass = 0;
	};
}
