#include "TextureSampler.h"

#include "graphics/GraphicsContext.h"

TextureSampler::TextureSampler(VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode) :
	sampler(GraphicsContext::LogicalDevice, vkDestroySampler, GraphicsContext::GlobalAllocator.Get())
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
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(GraphicsContext::LogicalDevice, &samplerInfo, sampler.AllocationCallbacks(), sampler.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

TextureSampler::~TextureSampler()
{
	sampler = nullptr;
}