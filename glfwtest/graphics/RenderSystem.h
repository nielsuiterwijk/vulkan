#pragma once

#include "RenderThread.h"
#include "ecs/SystemInterface.h"
#include "ecs/World.h"
#include "graphics/PipelineStateCache.h"
#include "graphics/models/Mesh.h"
#include "graphics/models/SubMesh.h"
#include "graphics/shaders/Material.h"
#include "graphics/textures/TextureSampler.h"


class RenderSystem // : Ecs::SystemInterface
{
public:
	virtual void Tick( Ecs::World& World, CommandBuffer* pCommandBuffer ) final
	{
		auto View = World.View<MeshComponent, MaterialComponent>();

		for ( auto& Entity : View )
		{
			MeshComponent& MeshData = View.Get<MeshComponent>( Entity );
			MaterialComponent& MaterialData = View.Get<MaterialComponent>( Entity );

			Material* pMaterial = MaterialData;
			if ( MaterialData._PipelineHash == 0 )
			{
				continue;
				MaterialData._PipelineHash = 1337;
			}

			VkPipeline Pipeline = PipelineStateCache::GetPipeline( MaterialData._PipelineHash );
			ASSERT( Pipeline );

			//if ( material == nullptr )
			//	return;
			//
			//if ( !material->IsLoaded() || !mesh->IsLoaded() || !TexturesLoaded() )
			//	return;


			//TODO: Create ubo for material
			//if ( mesh->GetMeshType() == MeshType::Skinned )
			//{
			//	std::shared_ptr<SkinnedMesh> pSkinnedMesh = std::static_pointer_cast<SkinnedMesh>( mesh );
			//
			//	auto localMeshUniformBuffer = new UniformBuffer( { &skinnedMeshBuffer, sizeof( SkinnedMeshBuffer ) } );
			//	material->AddUniformBuffer( localMeshUniformBuffer );
			//}

			pMaterial->UpdateUniformBuffers();
			//
			//if ( pso.IsDirty() )
			//{
			//	MeshData.BuildDescriptors( pMaterial );
			//
			//	pMaterial->GetSampler()->Initialize( VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, textures[ 0 ]->GetMipLevels() );
			//	pso.Create( pMaterial, std::vector<VkDynamicState>(), true );
			//	pso.SetVertexLayout( MeshData.bindingDescription, MeshData.attributeDescriptions );
			//	pso.Build( material );
			//}
			//
			vkCmdBindPipeline( pCommandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline );
			//
			for ( int i = 0; i < MeshData.subMeshes.size(); i++ )
			{
				//VkDescriptorSet set = pMaterial->AccessDescriptorPool().RetrieveDescriptorSet( material, textures[ i ].get(), material->GetSampler().get() );
				//vkCmdBindDescriptorSets( pCommandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pMaterial->AccessDescriptorPool().GetPipelineLayout(), 0, 1, &set, 0, nullptr );

				MeshData.subMeshes[ i ]->Draw( pCommandBuffer );
			}
		}
	}
};
