#include "BlockAllocator.h"

namespace
{
	void AllocateNewBlock( uint32_t Size, MemoryBlock& Block )
	{
		Block._LeftOver = Size;
		Block._Start = new unsigned char[ Size ];
		memset( Block._Start, 0x00, Size );
	}

	unsigned char* AllocateFromBlock( MemoryBlock& Block, uint32_t Size )
	{
		Block._LeftOver -= Size;
		return Block._Start + Block._LeftOver;
	}
}

BlockAllocator::BlockAllocator( uint32_t BlockSize )
	: Allocator()
	, _BlockSize( BlockSize )
{
}

BlockAllocator::~BlockAllocator()
{
	for ( MemoryBlock& Block : _Blocks )
	{
		delete[] Block._Start;
	}
	_NumAllocations = 0;
	_UsedMemory = 0;
}

void BlockAllocator::Initialize( size_t size, void* start )
{
	Allocator::Initialize( size, start );
}

void* BlockAllocator::Allocate( size_t Size, size_t Alignment )
{
	std::lock_guard<std::mutex> guard( _Mutex ); //NU TODO: Read/Write lock would be better..

	_NumAllocations++;
	_UsedMemory += Size;

	if ( _Blocks.empty() )
	{
		MemoryBlock MemoryBlock;
		AllocateNewBlock( _BlockSize, MemoryBlock );
		_Blocks.push_back( MemoryBlock );
	}

	for ( MemoryBlock& Block : _Blocks )
	{
		if ( Block._LeftOver > Size )
		{
			return AllocateFromBlock( Block, uint32_t( Size ) );
		}
	}

	MemoryBlock MemoryBlock;
	AllocateNewBlock( _BlockSize, MemoryBlock );
	_Blocks.push_back( MemoryBlock );

	return AllocateFromBlock( MemoryBlock, uint32_t( Size ) );
}

void BlockAllocator::Deallocate( void* address )
{
}
