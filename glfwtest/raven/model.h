#pragma once

#include "standard.h"
#include "graphics/shaders/Material.h"
#include "graphics/buffers/CommandBuffer.h"
#include "graphics/PipelineStateObject.h"
#include "graphics/buffers/UniformBuffer.h"
#include "graphics/buffers/UniformBufferDefinition.h"

class SkinnedMesh;


class Model
{
public:
	Model(const std::string& objectFile);
	~Model();


	void WindowResized(int w, int h);
	void Draw(std::shared_ptr<CommandBuffer> commandBuffer);

	std::shared_ptr<CameraUBO> GetUBO() const;
private:
	void FileLoaded(std::vector<char> fileData);

	bool TexturesLoaded() const;

private:
	PipelineStateObject pso;

	std::shared_ptr<CameraUBO> camera;
	std::shared_ptr<SkinnedMesh> mesh;
	std::shared_ptr<Material> material;

	std::vector<std::shared_ptr<Texture2D>> textures;
};