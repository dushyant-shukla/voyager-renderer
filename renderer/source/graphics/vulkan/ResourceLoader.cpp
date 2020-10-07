#include "ResourceLoader.h"
#include "utility/RendererCoreUtility.h"
#include "assertions.h"

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
}