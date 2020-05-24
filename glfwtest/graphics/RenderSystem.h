#pragma once

#include "RenderThread.h"
#include "ecs/SystemInterface.h"
#include "ecs/World.h"
#include "graphics/PipelineStateCache.h"
#include "graphics/models/Mesh.h"
#include "graphics/models/SubMesh.h"
#include "graphics/shaders/Material.h"


class RenderSystem // : Ecs::SystemInterface
{
public:
	virtual void Tick( Ecs::World& World, CommandBuffer& CommandBuffer ) final
	{
		auto View = World.View<MeshComponent, MaterialComponent, Texture2DComponent>();

		for ( auto& Entity : View )
		{
			MeshComponent& MeshData = View.Get<MeshComponent>( Entity );
			MaterialComponent& MaterialData = View.Get<MaterialComponent>( Entity );
			Texture2DComponent& TextureData = View.Get<Texture2DComponent>( Entity );

			Material* pMaterial = MaterialData;
			VkPipeline Pipeline = PipelineStateCache::GetPipeline( pMaterial->GetPipelineHash() );
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
			vkCmdBindPipeline( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline );
			
			/*
				 [ UNASSIGNED-CoreValidation-DrawState-PipelineLayoutsIncompatible ] Object: 0x9a90ce000000002b (Type = 23) | VkDescriptorSet 0x9a90ce000000002b[] bound as set #0 is not compatible with overlapping VkPipelineLayout 0xb3ec550000000090[] due to: 
				 DescriptorSetLayout 6BBCEA000000008F has 3 descriptors, but DescriptorSetLayout AB46AD0000000028, which comes from pipelineLayout, has 2 descriptors.
			*/

			for ( int i = 0; i < MeshData.subMeshes.size(); i++ )
			{
				VkDescriptorSet set = pMaterial->AccessDescriptorPool().RetrieveDescriptorSet( pMaterial, TextureData._TextureRefs[i].get() );
				vkCmdBindDescriptorSets( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pMaterial->AccessDescriptorPool().GetPipelineLayout(), 0, 1, &set, 0, nullptr );

				MeshData.subMeshes[ i ]->Draw( CommandBuffer );
			}
		}
	}
};
