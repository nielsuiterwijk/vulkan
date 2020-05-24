#pragma once

#include "graphics/PipelineStateObject.h"
#include "graphics/buffers/CommandBuffer.h"
#include "graphics/buffers/UniformBuffer.h"
#include "graphics/buffers/UniformBufferDefinition.h"
#include "graphics/shaders/Material.h"
#include "graphics/Camera.h"

#include "ecs/Entity.h"

class Mesh;
namespace Ecs
{
	class World;
}

class GameObject
{
public:
	GameObject( const std::string& objectFile );
	~GameObject();

	void WindowResized( int w, int h );

	void Update( );

	void Render( CommandBuffer* commandBuffer );

	Ecs::Entity CreateInstance( Ecs::World& World, int32_t Count = 1);


	std::shared_ptr<Mesh> GetMesh() const { return mesh; }

	const Camera::Buffer& GetUBO() const { return camera; }
	Camera::Buffer& AccessUBO() { return camera; }

	bool IsLoaded() const;

private:
	void FileLoaded( std::vector<char> fileData );

	bool TexturesLoaded() const;

private:
	//PipelineStateObject pso;
	VkPipeline _Pipeline = nullptr;

	Camera::Buffer camera;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

	std::vector<std::shared_ptr<Texture2D>> _Textures;
};
