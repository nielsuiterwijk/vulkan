#pragma once

#include "graphics/PipelineStateObject.h"
#include "graphics/buffers/CommandBuffer.h"
#include "graphics/buffers/UniformBuffer.h"
#include "graphics/buffers/UniformBufferDefinition.h"
#include "graphics/shaders/Material.h"
#include "graphics/Camera.h"

class SkinnedMesh;

class Model
{
public:
	Model( const std::string& objectFile );
	~Model();

	void WindowResized( int w, int h );

	void Update( );

	void Render( CommandBuffer* commandBuffer );

	const Camera::Buffer& GetUBO() const { return camera; }
	Camera::Buffer& AccessUBO() { return camera; }

private:
	void FileLoaded( std::vector<char> fileData );

	bool TexturesLoaded() const;

private:
	PipelineStateObject pso;

	Camera::Buffer camera;
	std::shared_ptr<SkinnedMesh> mesh;
	std::shared_ptr<Material> material;

	std::vector<std::shared_ptr<Texture2D>> textures;
};
