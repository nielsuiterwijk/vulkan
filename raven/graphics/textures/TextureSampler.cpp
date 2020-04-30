#include "TextureSampler.h"

#include "graphics/GraphicsContext.h"

TextureSampler::TextureSampler() :
	sampler( GraphicsContext::LogicalDevice, vkDestroySampler, GraphicsContext::GlobalAllocator.Get() )
{
}

TextureSampler::~TextureSampler()
{
	sampler = nullptr;
}

void TextureSampler::Initialize( VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, uint32_t mipLevels )
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

	if ( vkCreateSampler( GraphicsContext::LogicalDevice, &samplerInfo, sampler.AllocationCallbacks(), sampler.Replace() ) != VK_SUCCESS )
	{
		ASSERT_FAIL( "failed to create texture sampler!" );
	}
}
