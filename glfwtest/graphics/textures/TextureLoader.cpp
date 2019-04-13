#include "TextureLoader.h"

#include "Texture2D.h"

#include "graphics/buffers/VulkanBuffer.h"

#include "io/FileSystem.h"

#include "stb/stb_image.h"

TextureLoader::TextureLoader()
{
}

TextureLoader::~TextureLoader()
{
}

std::shared_ptr<Texture2D> TextureLoader::Get( const std::string& fileName )
{
	std::shared_ptr<Texture2D> texture2d = std::make_shared<Texture2D>();

	FileSystem::LoadFileAsync( "textures/" + fileName, std::bind( TextureLoader::FileLoaded, std::placeholders::_1, texture2d ) );

	return texture2d;
}

void TextureLoader::FileLoaded( std::vector<char> fileData, std::shared_ptr<Texture2D> texture2d )
{
	//unsigned char const *buffer

	unsigned char* pFileData = reinterpret_cast<unsigned char*>( fileData.data() );

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load_from_memory( pFileData, (int32_t)fileData.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	uint32_t mipLevels = static_cast<uint32_t>( std::floor( std::log2( std::max( texWidth, texHeight ) ) ) ) + 1;

	if ( !pixels )
	{
		throw std::runtime_error( "failed to load texture image!" );
	}

	uint32_t imageSizeWithMipMaps = imageSize * 1.33f;

	//VkBufferUsageFlags flags, BufferType::Enum bufferType, void* data, size_t size);
	VulkanBuffer buffer( VK_BUFFER_USAGE_TRANSFER_SRC_BIT, BufferType::Staging, pixels, imageSize );

	stbi_image_free( pixels );

	texture2d->AllocateImage( texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, ( VkImageUsageFlagBits )( VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
	texture2d->Transition( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ); //Setup image data so it can be transfered to.
	buffer.CopyStagingToImage( texture2d->GetImage(), texWidth, texHeight );
	texture2d->GenerateMipMaps();
	//texture2d->Transition(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); //transform from writeable to read only for shader
	texture2d->SetupView( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );
}
