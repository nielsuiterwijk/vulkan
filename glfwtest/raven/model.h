#pragma once

#include "standard.h"
#include "graphics/shaders/Material.h"
#include "graphics/buffers/CommandBuffer.h"
#include "graphics/PipelineStateObject.h"

class Model
{
private:
	std::shared_ptr<Material> material;

	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;

	PipelineStateObject pso;
};