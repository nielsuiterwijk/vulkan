#pragma once

#include "InstanceWrapper.h"

class GraphicsDevice
{
public:
	GraphicsDevice();

	~GraphicsDevice();

private:

	InstanceWrapper<VkInstance> applicationInfo { vkDestroyInstance };

};
