#include "Texture.h"

namespace vrassimp
{
	Texture::Texture() : texture(nullptr) {}

	Texture::Texture(Type _type, std::string _path)
		: type(_type), path(_path), texture(nullptr)
	{}

	Texture::~Texture()
	{
		if (texture != nullptr)
		{
			delete texture;
		}
	}
}