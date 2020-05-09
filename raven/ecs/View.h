#pragma once

#include <algorithm>
#include <tuple>
#include <type_traits>

namespace Ecs
{

#include "Storage.h"
#include "World.h"

	template <typename T, typename Tuple>
	struct has_type;

	//https://en.cppreference.com/w/cpp/types/disjunction
	//Basically does a logical OR, comparing each type defined in the template 'Us' to the type 'T'
	//https://stackoverflow.com/questions/25958259/how-do-i-find-out-if-a-tuple-contains-a-type
	template <typename T, typename... Us>
	struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...>
	{
	};

	template <class Entity, class... ComponentT>
	class View
	{
	private:
		using tuple_type = std::tuple<Storage<Entity, ComponentT> *...>;
		using underlying_iterator = typename SparseSet<Entity>::iterator;

	private:
		class Iterator
		{
			friend class View<Entity, ComponentT...>;

		public:
			using difference_type = typename underlying_iterator::difference_type;
			using value_type = typename underlying_iterator::value_type;
			using pointer = typename underlying_iterator::pointer;
			using reference = typename underlying_iterator::reference;
			using iterator_category = std::bidirectional_iterator_tag;

			Iterator( underlying_iterator It )
				: _It( It )
			{
			}

			Iterator operator++()
			{
				++_It;
				return *this;
			}

			bool operator==( const Iterator& Other ) const { return _It == Other._It; }
			bool operator!=( const Iterator& Other ) const { return !( *this == Other ); }

			pointer operator->() const { return _It.operator->(); }
			reference operator*() const { return *operator->(); }

		private:
			underlying_iterator _It;
		};

	public:
		View( World& World, Storage<Entity, ComponentT>&... ComponentStorage )
			: _World( World )
			, _Pools { &ComponentStorage... }
		{
			//At this point, our _Pools tuple holds all ptr's to the component storage.
			//We have to build a minimal list of entities that we can iterate over.
			//Which of course is always the storage with the LEAST amount of components stored in it.

			const auto& MinSet = GetMinimalSet();
			int32_t Size = MinSet.GetSize();
		}

		template <typename... ComponentT>
		decltype( auto ) Get( const Entity Entity_ ) const
		{
			//ASSERT( Contains( Entity ) );

			if constexpr ( sizeof...( ComponentT ) == 0 )
			{
				static_assert( sizeof...( ComponentT ) == 1 );
				return ( std::get<Storage<Entity, ComponentT>*>( _Pools )->Get( Entity_ ), ... );
			}
			else if constexpr ( sizeof...( ComponentT ) == 1 )
			{
				static_assert( has_type<Storage<Entity, ComponentT>*..., tuple_type>::value, "View does not contain type" );
				return ( std::get<Storage<Entity, ComponentT>*>( _Pools )->Get( Entity_ ), ... );
			}
			else
			{
				//static_assert( std::is_reference<decltype( Get<ComponentT>( {} ) )...>::value, "When asking for multiple components, they have to be references" );
				return std::tuple<decltype( Get<ComponentT>( {} ) )...> { Get<ComponentT>( Entity_ )... };
			}
		}

		//Returns an iterator starting at the beginning. The iterator uses the minimal set's dense container.
		Iterator begin() const
		{
			const SparseSet<Entity>& MinSet = GetMinimalSet();
			return Iterator { MinSet.begin() };
		}

		Iterator end() const
		{
			const SparseSet<Entity>& MinSet = GetMinimalSet();
			return Iterator { MinSet.end() };
		}

	private:
		const SparseSet<Entity>& GetMinimalSet() const
		{
			auto Compare = []( const SparseSet<Entity>* Left, const SparseSet<Entity>* Right ) { return Left->GetSize() < Right->GetSize(); };

			//We transition our tuple into an initializer list by unpacking and casting each datatype to it's base type
			auto Result = std::min( { static_cast<const SparseSet<Entity>*>( std::get<Storage<Entity, ComponentT>*>( _Pools ) )... }, Compare );
			return *Result;
		}

	private:
		World& _World;

		const tuple_type _Pools; //todo: better name -> _StoragePerComponent ?
	};

};