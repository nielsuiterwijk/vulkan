#pragma once

#include <algorithm>
#include <tuple>

namespace Ecs
{

#include "Storage.h"
#include "World.h"

	template <class Entity, class... ComponentT>
	class View
	{
	private:
		//using FilterType = std::array<
	public:
		using DataType = int32_t;
		class Iterator
		{
		public:
			Iterator( DataType* ptr )
				: ptr( ptr )
			{
			}
			Iterator operator++()
			{
				++ptr;
				return *this;
			}
			bool operator!=( const Iterator& other ) const { return ptr != other.ptr; }
			const DataType& operator*() const { return *ptr; }

		private:
			DataType* ptr;
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

	private:
		const SparseSet<Entity>& GetMinimalSet()
		{
			auto Compare = []( const auto* Left, const auto* Right ) { return Left->GetSize() < Right->GetSize(); };

			//We transition our tuple into an initializer list by unpacking and casting each datatype to it's base type
			auto Result = std::min( { static_cast<const SparseSet<Entity>*>( std::get<Storage<Entity, ComponentT>*>( _Pools ) )... }, Compare );
			return *Result;
		}

	private:
		World& _World;

		//const std::tuple<pool_type<Component>*..., pool_type<Exclude>*...> pools;
		const std::tuple<Storage<Entity, ComponentT>*...> _Pools;
	};

};