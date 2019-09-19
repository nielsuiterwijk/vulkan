#pragma once

#include "graphics/helpers/InstanceWrapper.h"
#include "standard.h"

//TODO: These probably should be cached
class TextureSampler
{
public:
	explicit TextureSampler();
	~TextureSampler();

	void Initialize( VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, uint32_t mipLevels );

	VkSampler GetNative() const { return sampler; }

private:
	InstanceWrapper<VkSampler> sampler;
};
