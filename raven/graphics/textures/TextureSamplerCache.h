#pragma once


class TextureSamplerCache
{
public:
	static uint32_t CalcHash( VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, uint32_t mipLevels );

	static VkSampler GetOrCreateSampler( VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, uint32_t mipLevels );
	static VkSampler GetSampler( VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, uint32_t mipLevels ); //Returns nullptr on missing
	static VkSampler GetSampler( uint32_t Hash ); //Returns nullptr on missing
	static void Destroy();
};