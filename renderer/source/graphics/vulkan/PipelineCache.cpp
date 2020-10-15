#include "PipelineCache.h"
#include "RendererState.h"
#include "utility/RendererCoreUtility.h"

namespace vr
{
	PipelineCache::PipelineCache()
	{
	}

	PipelineCache::~PipelineCache()
	{
		if (mCache != VK_NULL_HANDLE)
		{
			vkDestroyPipelineCache(LOGICAL_DEVICE, mCache, ALLOCATION_CALLBACK);
		}
	}

	void PipelineCache::Setup()
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		CHECK_RESULT(vkCreatePipelineCache(LOGICAL_DEVICE, &pipelineCacheCreateInfo, ALLOCATION_CALLBACK, &mCache), "RESOURCE CREATION FAILED: PIPELINE CACHE");
		RENDERER_DEBUG("RESOURCE CREATED: PIPELINE CACHE");
	}
}