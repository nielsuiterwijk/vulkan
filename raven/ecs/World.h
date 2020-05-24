#pragma once

#include <memory>
#include <unordered_map>

#include "ComponentInterface.h"
#include "Entity.h"
#include "Storage.h"
#include "View.h"


/*
API:

Entity* pBall = World.Create();

pBall.Uses<Position>( { 0, 0, 0 } );
pBall.Uses<Gravity>( { -9.81f } );

BounceSystem Bouncing();

World.Register(&Bouncing);

World.Tick() ->
	Bouncing.Tick(World& World)
	{
		Iterator<Position> Start = World.Access<Position>();

		while (Start)
		{
			Position& Pos = *Start++;
		}
	}






*/

namespace Ecs
{
	//Can create & destroy entities
	//Will update systems

	//Contains all components
	//TODO: Allow for user allocations?

	class AllocatorInterface
	{
	public:
		virtual void Initialize() = 0;

		virtual void* Allocate( size_t size, size_t alignment = 1 ) = 0;
		virtual void Deallocate( void* pAddress ) = 0;
	};

	struct PoolWrapper
	{
		IdType _ComponentId;
		std::unique_ptr<Ecs::SparseSet<Entity>> _Pool;
	};


	class World
	{
	private:
		using EntityType = Entity;
		using TraitsType = Traits<std::underlying_type_t<Entity>>;

		AllocatorInterface* pEntityAllocator = nullptr;
		AllocatorInterface* pComponentPoolAllocator = nullptr;

	public:
		World() = default;
		World( World& ) = delete;

	private:
		template <class T>
		T* AllocateEntity()
		{
			if ( pEntityAllocator != nullptr )
			{
				void* pDest = pEntityAllocator->Allocate( sizeof( T ) );
				return new ( pDest ) T( *this );
			}
			else
			{
				return new T( *this );
			}
		}

	public:
		template <typename ComponentT>
		const ComponentT* raw() const
		{
			return GetOrCreatePool<ComponentT>().GetData();
		}


	public:
		EntityType Create( int32_t Num = 1 )
		{
			//TODO: Should allow for reuse of destroyed entities
			return _Entities.emplace_back( static_cast<EntityType>( _Entities.size() ) );
		}

		void End()
		{
			_Storage.clear();
			_Entities.clear();
		}

		bool IsValid( const EntityType Entity ) const
		{
			const IdType Index = ToPrimitive( Entity ) & TraitsType::index_mask;
			return Index < _Entities.size() && _Entities[ Index ] == Entity;
		}

		template <typename ComponentT, typename... ArgsT>
		ComponentT& Assign( const EntityType Entity, ArgsT&&... Args )
		{
			ASSERT( IsValid( Entity ) );
			Storage<EntityType, ComponentT>& Storage = GetOrCreatePool<ComponentT>(); // .emplace( *this, entity, std::forward<Args>( args )... );

			return Storage.Emplace( Entity, std::forward<ArgsT>( Args )... );
		}

		template <typename ComponentT>
		ComponentT& Get( const EntityType Entity )
		{
			ASSERT( IsValid( Entity ) );
			Storage<EntityType, ComponentT>& Storage = GetOrCreatePool<ComponentT>(); // .emplace( *this, entity, std::forward<Args>( args )... );

			return Storage.Get( Entity );
		}

		template <typename ComponentT>
		Ecs::Storage<EntityType, ComponentT>& GetOrCreatePool() const
		{
			static_assert( std::is_same_v<ComponentT, std::decay_t<ComponentT>> );

			if constexpr ( has_type_index_v<ComponentT> )
			{
				const IndexType Index = GetTypeIndex<ComponentT>::value();
				const IdType Id = GetTypeInfo<ComponentT>::id();


				auto It = _Storage.find( Id );

				if ( It != _Storage.end() )
				{
					return static_cast<Storage<EntityType, ComponentT>&>( *It->second._Pool );
				}
				else
				{
					std::cout << "Component: " << Index << " with id: " << Id << std::endl;

					PoolWrapper& Pool = _Storage[ Id ];
					Pool._ComponentId = Id;
					Pool._Pool.reset( new Storage<EntityType, ComponentT>() );
					return static_cast<Storage<EntityType, ComponentT>&>( *Pool._Pool );
				}
			}
			else
			{
				ASSERT_FAIL( "Component does not have type available" );

				return nullptr;
			}
		}


		template <class... ComponentsT>
		Ecs::View<EntityType, ComponentsT...> View()
		{
			static_assert( sizeof...( ComponentsT ) > 0 );
			return { *this, GetOrCreatePool<ComponentsT>()... };
		}

	private:
		//TODO: Implement own map for fun.

		std::vector<EntityType> _Entities;
		mutable std::unordered_map<IdType, PoolWrapper> _Storage;
	};
};