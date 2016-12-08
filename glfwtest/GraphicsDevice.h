#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <memory>

class VulkanRenderer;

struct QueueFamilyIndices
{
	int graphicsFamily = -1;

	bool IsComplete()
	{
		return graphicsFamily >= 0;
	}
};

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	void Initialize(std::shared_ptr<VulkanRenderer> vulkanRenderer);

private:
	void CreateDevice();
	QueueFamilyIndices FindQueueFamilies();

private:
	std::shared_ptr<VulkanRenderer> renderer;

	VkPhysicalDevice device;

};
