#pragma once

#include "Allocator.h"

class FreeListAllocator : public Allocator
{
public:
	FreeListAllocator();
	~FreeListAllocator();

	void Initialize( size_t size, void* start );

	void* Allocate( size_t size, size_t alignment = 0 ) override;
	void Deallocate( void* address ) override;

	size_t GetSizeOf( void* address ) override;
	size_t GetAlignmentOf( void* address ) override;

private:
	struct AllocationHeader
	{
		size_t size;
		uint16_t alignment;
		uint16_t adjustment;
	};

	struct FreeBlock
	{
		size_t size;
		FreeBlock* next;
	};

	FreeListAllocator( const FreeListAllocator& ); // Prevent copies because it might cause errors
	FreeListAllocator& operator=( const FreeListAllocator& );

	FreeBlock* freeBlocks;
};
