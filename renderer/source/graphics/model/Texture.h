#pragma once

#include <string>

#include "graphics/vulkan/VulkanTexture.h"

namespace vrassimp
{
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
}