#pragma once

#include "standard.h"
#include "graphics/helpers/InstanceWrapper.h"


class Texture2D
{
public: 
	Texture2D();
	virtual ~Texture2D();

	void AllocateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling imageTiling, VkImageUsageFlagBits imageUsage, VkMemoryPropertyFlagBits propertyFlags);

	void SetImage(const VkImage& image);

	void SetupView(VkFormat format, VkImageAspectFlags aspectFlags);

	void Transition(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	VkImageView GetImageView() { return imageView; }

	VkFormat GetFormat() const { return format; }

protected:
	InstanceWrapper<VkImage> image;
	InstanceWrapper<VkImageView> imageView;
	InstanceWrapper<VkDeviceMemory> imageDeviceMemory;

	uint32_t width;
	uint32_t height;

	VkFormat format;

};