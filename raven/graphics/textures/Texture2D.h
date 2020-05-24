#pragma once

#include "graphics/helpers/InstanceWrapper.h"
#include "graphics/memory/GPUAllocator.h"
#include "vma/vk_mem_alloc.h"


class Texture2D
{
public:
	Texture2D();
	virtual ~Texture2D();

	void AllocateImage( uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling imageTiling, VkImageUsageFlagBits imageUsage, VmaMemoryUsage Usage );

	void SetupView( VkFormat format, VkImageAspectFlags aspectFlags );

	void Transition( VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout );

	void GenerateMipMaps();

	VkFormat GetFormat() const { return _Format; }
	VkImage GetImage() const { return _Resource.Image; }
	VkImageView GetImageView() const { return _View; }

	uint32_t GetMipLevels() const { return _MipLevels; }

	bool IsLoaded() const;

private:
protected:
	InstanceWrapper<VkImageView> _View;

	SAllocatedImage _Resource;

	uint32_t _MipLevels;
	uint32_t _Width;
	uint32_t _Height;

	VkFormat _Format;
};

struct Texture2DComponent
{
	std::vector<std::shared_ptr<Texture2D>> _TextureRefs;
};