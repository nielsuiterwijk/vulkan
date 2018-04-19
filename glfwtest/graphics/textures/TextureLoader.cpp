#include "TextureLoader.h"

#include "io/FileSystem.h"

#include "stb/stb_image.h"


TextureLoader::TextureLoader()
{

}

TextureLoader::~TextureLoader()
{

}

std::shared_ptr<Texture2D> TextureLoader::Get(const std::string& fileName)
{
	FileSystem::LoadFileAsync("textures/" + fileName, &TextureLoader::FileLoaded);

	return nullptr;
}

void TextureLoader::FileLoaded(std::vector<char> fileData)
{
	//unsigned char const *buffer

	unsigned char* pFileData = reinterpret_cast<unsigned char*>(fileData.data());

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load_from_memory(pFileData, fileData.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}
}