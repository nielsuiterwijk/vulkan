#pragma once

#include "standard.h"
#include "graphics/helpers/InstanceWrapper.h"

//TODO: These probably should be cached
class TextureSampler
{
public:
	TextureSampler(VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode);
	~TextureSampler();


	VkSampler GetNative() const { return sampler; }

private:
	InstanceWrapper<VkSampler> sampler;



};