#include "Mesh.h"

#include "graphics/buffers/VulkanBuffer.h"
#include "graphics/shaders/Material.h"
#include "graphics/buffers/UniformBuffer.h"
#include "graphics/models/SubMesh.h"

#include "helpers/Timer.h"
#include "tinyobj/tiny_obj_loader.h"


Mesh::Mesh() :
	triangleCount(0),
	vertexFormatFlags(0)
{		

}

Mesh::~Mesh()
{
	for (int i = 0; i < subMeshes.size(); i++)
	{
		delete subMeshes[i];
	}

	subMeshes.clear();
}

bool Mesh::AllocateBuffers(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize, uint32_t triangles)
{
	SubMesh* subMesh = new SubMesh(triangles);

	subMesh->AllocateBuffers(vertexData, vertexDataSize, indexData, indexDataSize);

	subMeshes.emplace_back(subMesh);


	return true;
}

void Mesh::SetupCommandBuffer(std::shared_ptr<CommandBuffer> commandBuffer, const PipelineStateObject& pso, std::shared_ptr<Material> material) const
{	
	vkCmdBindPipeline(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());

	//https://www.khronos.org/registry/vulkan/specs/1.0/man/html/vkCmdBindDescriptorSets.html

	VkDescriptorSet set = GraphicsContext::DescriptorPool->GetDescriptorSet(material->GetUniformBuffers()[0], material->GetTexture().get(), material->GetSampler().get());
	vkCmdBindDescriptorSets(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsContext::PipelineLayout, 0, 1, &set, 0, nullptr);

	for (int i = 0; i < subMeshes.size(); i++)
	{
		subMeshes[i]->Draw(commandBuffer);
	}


}


bool Mesh::IsLoaded() const
{
	for (int i = 0; i < subMeshes.size(); i++)
	{
		if (!subMeshes[i]->IsLoaded())
			return false;
	}	

	return true;
}
