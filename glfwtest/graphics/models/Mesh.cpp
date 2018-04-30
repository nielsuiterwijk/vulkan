#include "Mesh.h"

#include "graphics/buffers/VulkanBuffer.h"
#include "graphics/shaders/Material.h"
#include "graphics/buffers/UniformBuffer.h"

#include "helpers/Timer.h"
#include "tinyobj/tiny_obj_loader.h"

Mesh::Mesh() :
	triangleCount(0),
	vertexFormatFlags(0),
	indexBuffer(nullptr),
	vertexBuffer(nullptr)
{		

}

Mesh::~Mesh()
{
	delete vertexBuffer;
	delete indexBuffer;
}

bool Mesh::AllocateBuffers(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize)
{
	assert(vertexBuffer == nullptr);
	vertexBuffer = new VulkanBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, BufferType::Static, vertexData, vertexDataSize);

	assert(indexBuffer == nullptr);
	indexBuffer = new VulkanBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, BufferType::Static, indexData, indexDataSize);

	return true;
}

void Mesh::SetupCommandBuffer(std::shared_ptr<CommandBuffer> commandBuffer, const PipelineStateObject& pso, std::shared_ptr<Material> material) const
{	
	vkCmdBindPipeline(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());

	VkBuffer vertexBuffers[] = { vertexBuffer->GetNative() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer->GetNative(), 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer->GetNative(), indexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT32);
	
	VkDescriptorSet set = GraphicsContext::DescriptorPool->GetDescriptorSet(material->GetUniformBuffers()[0], material->GetTexture().get(), material->GetSampler().get());

	//https://www.khronos.org/registry/vulkan/specs/1.0/man/html/vkCmdBindDescriptorSets.html
	vkCmdBindDescriptorSets(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsContext::PipelineLayout, 0, 1, &set, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer->GetNative(), triangleCount * 3, 1, 0, 0, 0);

}
