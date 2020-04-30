#pragma once

#include "Types.h"

namespace Ecs
{
	class World;

	template <typename>
	struct Traits;

	template <>
	struct Traits<uint64_t>
	{
		/*! @brief Underlying entity type. */
		using entity_type = uint64_t;
		/*! @brief Underlying version type. */
		using version_type = uint32_t;
		/*! @brief Difference type. */
		using difference_type = int64_t;

		/*! @brief Mask to use to get the entity number out of an identifier. */
		static constexpr uint64_t entity_mask = 0xFFFFFFFF;
		/*! @brief Mask to use to get the version out of an identifier. */
		static constexpr uint64_t version_mask = 0xFFFFFFFF;
		/*! @brief Extent of the entity number within an identifier. */
		static constexpr auto entity_shift = 32;
	};

	//Null entity, or otherwise invalid
	class Null
	{
		template <typename Entity>
		using TraitsType = Traits<std::underlying_type_t<Entity>>;

	public:
		template <typename Entity>
		constexpr operator Entity() const
		{
			return Entity { TraitsType<Entity>::entity_mask };
		}

		constexpr bool operator==( Null ) const { return true; }

		constexpr bool operator!=( Null ) const { return false; }

		template <typename Entity>
		constexpr bool operator==( const Entity entity ) const
		{
			return ( ToPrimitive( entity ) & TraitsType<Entity>::entity_mask ) == ToPrimitive( static_cast<Entity>( *this ) );
		}

		template <typename Entity>
		constexpr bool operator!=( const Entity entity ) const
		{
			return !( entity == *this );
		}
	};

	template <typename Entity>
	constexpr bool operator==( const Entity entity, Null other )
	{
		return other.operator==( entity );
	}


	template <typename Entity>
	constexpr bool operator!=( const Entity entity, Null other )
	{
		return !( other == entity );
	}


	//Note: We are using the enum class instead of `using Entity = IdType;` to prevent operations such as += etc, an Entity is just some identifier
	enum class Entity : IdType
	{
	};
	//This can be used to cast the enum class to its primitive type
	constexpr IdType ToPrimitive( const Entity id ) { return static_cast<std::underlying_type_t<Entity>>( id ); }


	/*class Entity
	{

	public:
		Entity() = delete;
		Entity( World& World );

		template <typename ComponentT>
		ComponentT& Uses()
		{
			return Uses< ComponentT>( ComponentT() );
		}
		
		template <typename ComponentT>
		ComponentT& Uses( ComponentT& Default )
		{
			static_assert( std::is_same_v<ComponentT, std::decay_t<ComponentT>> );

			if constexpr ( has_type_index_v<ComponentT> )
			{
				const IndexType Index = GetTypeIndex<ComponentT>::value();
				const IdType Id = GetTypeInfo<ComponentT>::id();

				std::cout << "Component: " << Index << " with id: " << Id << std::endl;
			}
			else
			{
				ASSERT_FAIL( "Component does not have type available" );
			}

			return Default;
		}


	private:
		//World& _World;
	};*/
}
