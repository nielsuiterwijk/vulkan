#pragma once

#include <vector>

namespace Ecs
{
	/*
		Idea of the sparse set is to keep a dense array and a sparse array. The dense one can be used to access data efficiently, 
		And a sparse one that keeps track of indices into the dense array

		This variant works with a 'page' model, where the _Sparse keeps track of entities per page to reduce the search window.
	*/
	template <typename EntityT>
	class SparseSet
	{
	private:
		static constexpr int32_t PageSizeInBytes = 4096;

		using TraitsType = Traits<std::underlying_type_t<EntityT>>;

		static_assert( PageSizeInBytes && ( ( PageSizeInBytes & ( PageSizeInBytes - 1 ) ) == 0 ) ); //pow2
		static constexpr auto EntitiesPerPage = PageSizeInBytes / sizeof( typename TraitsType::entity_type );

	private:
		uint32_t CalcPageIndex( const EntityT Entity ) const { return uint32_t { ( ToPrimitive( Entity ) & TraitsType::entity_mask ) / EntitiesPerPage }; }

		uint32_t CalcOffset( const EntityT Entity ) const { return uint32_t { ToPrimitive( Entity ) & ( EntitiesPerPage - 1 ) }; }

		EntityT* GetOrCreatePage(uint32_t PageIndex)
		{
			if ( PageIndex >= _Sparse.size() )
			{
				_Sparse.resize( PageIndex + 1 );
			}

			if ( _Sparse[ PageIndex ] == nullptr )
			{
				_Sparse[ PageIndex ] = std::make_unique<EntityT[]>( EntitiesPerPage );

				//Default initialize our array with Null entities
				std::fill_n( _Sparse[ PageIndex ].get(), EntitiesPerPage, NullEntity );
			}

			return _Sparse[ PageIndex ].get();
		}

	public:
		virtual int32_t GetSize() const { return -1; }

		void Emplace( const EntityT Entity )
		{
			ASSERT( !Contains( Entity ) );

			const uint32_t PageIndex = CalcPageIndex( Entity );

			GetOrCreatePage( PageIndex )[ CalcOffset( Entity ) ] = EntityT( _Dense.size() );

			_Dense.emplace_back( Entity );
		}

		bool Contains( const EntityT Entity ) const
		{
			const uint32_t PageIndex = CalcPageIndex( Entity );
			// testing against null so we don't have to access the dense array, however the version might of course miss match, but the storage does not care/know about that.
			return ( PageIndex < _Sparse.size() && _Sparse[ PageIndex ] && _Sparse[ PageIndex ][ CalcOffset( Entity ) ] != NullEntity );
		}

	private:
		std::vector<EntityT> _Dense;
		std::vector<std::unique_ptr<EntityT[]>> _Sparse; //Contains pages, each page contains the indices that map into the _Dense array
	};

	template <typename EntityT, typename ComponentT>
	class Storage : public SparseSet<EntityT>
	{
		using BaseType = SparseSet<EntityT>;
	public:
		template <typename... ArgsT>
		ComponentT& Emplace( EntityT Entity, ArgsT&&... Args )
		{
			BaseType::Emplace( Entity );
			return _Container.emplace_back( std::forward<ArgsT>( Args )... );
		}

		const ComponentT* GetData() const { return _Container.data(); }

		int32_t GetSize() const final { return static_cast<int32_t>( _Container.size() ); }

		std::vector<ComponentT> _Container;
	};

};
