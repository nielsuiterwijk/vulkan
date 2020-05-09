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
		template <class Entity, class... ComponentT>
		friend class View;

		using TraitsType = Traits<std::underlying_type_t<EntityT>>;
		using DenseMap = std::vector<EntityT>;

	private:
		class Iterator final
		{
			friend class SparseSet<EntityT>;

			using index_type = typename TraitsType::difference_type;

			Iterator( const DenseMap& ref, const index_type Index )
				: _DenseRef { ref }
				, _Index { Index }
			{
			}

		public:
			using difference_type = index_type;
			using value_type = EntityT;
			using pointer = const value_type*;
			using reference = const value_type&;
			using iterator_category = std::random_access_iterator_tag;

			Iterator() = default;

			Iterator& operator++() { return ++_Index, *this; }

			Iterator operator++( int )
			{
				iterator Copy = *this;
				return operator++(), Copy;
			}

			Iterator& operator--() { return --_Index, *this; }

			Iterator operator--( int )
			{
				Iterator Copy = *this;
				return operator--(), Copy;
			}

			Iterator& operator+=( const difference_type Value )
			{
				_Index += Value;
				return *this;
			}

			Iterator operator+( const difference_type Value ) const
			{
				Iterator Copy = *this;
				return ( Copy -= Value );
			}

			Iterator& operator-=( const difference_type Value ) { return ( *this += -Value ); }

			Iterator operator-( const difference_type Value ) const { return ( *this + -Value ); }

			difference_type operator-( const Iterator& Other ) const { return Other._Index - _Index; }

			reference operator[]( const difference_type Value ) const
			{
				const int32_t Pos = _Index - Value - 1;
				return _DenseRef[ Pos ];
			}

			bool operator==( const Iterator& Other ) const { return Other._Index == _Index; }

			bool operator!=( const Iterator& Other ) const { return !( *this == Other ); }

			bool operator<( const Iterator& Other ) const { return _Index > Other._Index; }

			bool operator>( const Iterator& Other ) const { return _Index < Other._Index; }

			bool operator<=( const Iterator& Other ) const { return !( *this > Other ); }

			bool operator>=( const Iterator& Other ) const { return !( *this < Other ); }

			pointer operator->() const
			{
				return &_DenseRef[ _Index ];
			}

			reference operator*() const { return *operator->(); }

		private:
			const DenseMap& _DenseRef;
			index_type _Index;
		};

	private:
		static constexpr int32_t PageSizeInBytes = 4096;

		static_assert( PageSizeInBytes && ( ( PageSizeInBytes & ( PageSizeInBytes - 1 ) ) == 0 ) ); //pow2
		static constexpr auto EntitiesPerPage = PageSizeInBytes / sizeof( typename TraitsType::entity_type );


	private:
		uint32_t CalcPageIndex( const EntityT Entity ) const { return uint32_t { ( ToPrimitive( Entity ) & TraitsType::index_mask ) / EntitiesPerPage }; }

		uint32_t CalcOffset( const EntityT Entity ) const { return uint32_t { ToPrimitive( Entity ) & ( EntitiesPerPage - 1 ) }; }

		EntityT* GetOrCreatePage( uint32_t PageIndex )
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
		using iterator = Iterator;
		using SizeType = uint32_t;

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

		SizeType LookUp( const EntityT Entity ) const
		{
			ASSERT( Contains( Entity ) );
			return SizeType( _Sparse[ CalcPageIndex( Entity ) ][ CalcOffset( Entity ) ] );
		}

		//Returns an iterator starting at the beginning. The iterator uses the minimal set's dense container.
		Iterator begin() const { return Iterator { _Dense, 0 }; }

		Iterator end() const { return Iterator { _Dense, static_cast<TraitsType::difference_type>( _Dense.size() ) }; }

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

		const ComponentT& Get( const EntityT Entity ) const { return _Container[ BaseType::LookUp( Entity ) ]; }
		ComponentT& Get( const EntityT Entity ) { return _Container[ BaseType::LookUp( Entity ) ]; }


		int32_t GetSize() const final { return static_cast<int32_t>( _Container.size() ); }



	private:
		std::vector<ComponentT> _Container;
	};

};
