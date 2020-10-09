#include "ResourceLoader.h"
#include "utility/RendererCoreUtility.h"
#include "assertions.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>

namespace vr
{
	std::vector<char> ResourceLoader::ReadFile(const std::string& filename)
	{
		// open stream from given file
		// std::ios::binary tells stream to read file as binary
		// std::ios::ate tells stream to start reading from end of file
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (!file.is_open())
		{
			THROW("FAILED TO OPEN FILE: " + filename);
		}

		// get current read position and use to resize file buffer
		size_t filesize = (size_t)file.tellg();
		std::vector<char> buffer(filesize);

		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();

		return buffer;
	}

	stbi_uc* ResourceLoader::LoadTextureFile(const std::string filename, int* const width, int* const height, VkDeviceSize* const imageSize)
	{
		int channels;

		std::string filelocation = "../../assets/textures/" + filename;
		stbi_uc* image = stbi_load(filelocation.c_str(), width, height, &channels, STBI_rgb_alpha);

		if (!image)
		{
			throw std::runtime_error("Failed to load a texture file: " + filelocation);
		}

		*imageSize = (*width) * (*height) * 4;
		return image;
	}
}