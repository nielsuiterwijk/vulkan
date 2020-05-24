#include "TextureSamplerCache.h"

#include "graphics/GraphicsContext.h"
#include "helpers/Murmur3.h"

static std::unordered_map<uint32_t, VkSampler> Cache;

uint32_t TextureSamplerCache::CalcHash( VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, uint32_t mipLevels )
{
	uint32_t Hash = 0;
	Hash = Murmur3::Hash( static_cast<uint32_t>( min ), Hash );
	Hash = Murmur3::Hash( static_cast<uint32_t>( mag ), Hash );
	Hash = Murmur3::Hash( static_cast<uint32_t>( mipmapMode ), Hash );
	Hash = Murmur3::Hash( static_cast<uint32_t>( addressMode ), Hash );
	Hash = Murmur3::Hash( mipLevels, Hash );

	ASSERT( Hash != 0 );

	return Hash;
}

VkSampler TextureSamplerCache::GetOrCreateSampler( VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, uint32_t mipLevels )
{
	uint32_t Hash = CalcHash( min, mag, mipmapMode, addressMode, mipLevels );

	VkSampler Result = GetSampler( Hash );

	if ( Result == nullptr )
	{
		//https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkSamplerCreateInfo.html
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = min;
		samplerInfo.minFilter = mag;
		samplerInfo.addressModeU = addressMode;
		samplerInfo.addressModeV = addressMode;
		samplerInfo.addressModeW = addressMode;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>( mipLevels );

		if ( vkCreateSampler( GraphicsContext::LogicalDevice, &samplerInfo, GraphicsContext::LocalAllocator, &Result ) != VK_SUCCESS )
		{
			ASSERT_FAIL( "failed to create texture sampler!" );
		}

		Cache[ Hash ] = Result;
	}

	return Result;
}


VkSampler TextureSamplerCache::GetSampler( VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, uint32_t mipLevels )
{
	uint32_t Hash = CalcHash( min, mag, mipmapMode, addressMode, mipLevels );

	return GetSampler( Hash );
}

VkSampler TextureSamplerCache::GetSampler( uint32_t Hash )
{
	auto It = Cache.find( Hash );
	if ( It != Cache.end() )
	{
		return It->second;
	}
	return nullptr;
}

void TextureSamplerCache::Destroy()
{
	for ( auto& it : Cache )
	{
		vkDestroySampler( GraphicsContext::LogicalDevice, it.second, GraphicsContext::LocalAllocator );
	}
	Cache.clear();
}