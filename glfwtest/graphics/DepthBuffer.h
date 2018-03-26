#pragma once

#include "textures/Texture2D.h"

class DepthBuffer : public Texture2D
{
public:
	DepthBuffer();
	virtual ~DepthBuffer();

	void Destroy();
	
	void Initialize(uint32_t width, uint32_t height);

private:
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();

private:

};