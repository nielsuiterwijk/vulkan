#pragma once

#include "Allocator.h"

struct MemoryBlock
{
	uint32_t _LeftOver;
	unsigned char* _Start;
};

//Cheap allocations, free deallocation. It won't actually reuse the memory. Usage is for allocations that are generally done for the lifetime of the program.
//It will allocate 'blocks' based on the BlockSize
class BlockAllocator : public Allocator
{
public:
	BlockAllocator( uint32_t BlockSize );
	~BlockAllocator();

	BlockAllocator( const BlockAllocator& ) = delete;
	BlockAllocator& operator=( const BlockAllocator& ) = delete;

	void Initialize( size_t size, void* start );

	void* Allocate( size_t size, size_t alignment = 0 ) override;
	void Deallocate( void* address ) override;

	size_t GetSizeOf( void* address ) override { return 0; };
	size_t GetAlignmentOf( void* address ) override { return 1; };

private:
	uint32_t _BlockSize;

	std::vector<MemoryBlock> _Blocks;
};
