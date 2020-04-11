#include "FreeListAllocator.h"

FreeListAllocator::FreeListAllocator() :
	freeBlocks( nullptr )
{
}

FreeListAllocator::~FreeListAllocator()
{
	freeBlocks = nullptr;
}

void FreeListAllocator::Initialize( size_t size, void* start )
{
	Allocator::Initialize( size, start );
	assert( size > sizeof( FreeBlock ) );

	freeBlocks = (FreeBlock*)start;

	freeBlocks->size = size;
	freeBlocks->next = nullptr;
}

void* FreeListAllocator::Allocate( size_t size, size_t alignment )
{
	assert( size != 0 );

	std::lock_guard<Mutex> guard( _Mutex );

	FreeBlock* previousBlock = nullptr;
	FreeBlock* currentBlock = freeBlocks;

	while ( currentBlock != nullptr )
	{
		// Calculate adjustment needed to keep object correctly aligned
		size_t adjustment = Util::GetAlignedPtrAdjustment( currentBlock, alignment, sizeof( AllocationHeader ) );

		size_t totalSize = size + adjustment;

		// If allocation doesn't fit in this FreeBlock, try the next
		if ( currentBlock->size < totalSize )
		{
			previousBlock = currentBlock;
			currentBlock = currentBlock->next;
			continue;
		}

		static_assert( sizeof( AllocationHeader ) >= sizeof( FreeBlock ), "sizeof(AllocationHeader) < sizeof(FreeBlock)" );

		// If allocations in the remaining memory will be impossible
		if ( currentBlock->size - totalSize <= sizeof( AllocationHeader ) )
		{
			// Increase allocation size instead of creating a new FreeBlock
			totalSize = currentBlock->size;

			if ( previousBlock != nullptr )
			{
				previousBlock->next = currentBlock->next;
			}
			else
			{
				freeBlocks = currentBlock->next;
			}
		}
		else
		{
			// Else create a new FreeBlock containing remaining memory
			FreeBlock* nextBlock = (FreeBlock*)Util::AddPtr( currentBlock, (int)totalSize );
			nextBlock->size = currentBlock->size - totalSize;
			nextBlock->next = currentBlock->next;

			if ( previousBlock != nullptr )
			{
				previousBlock->next = nextBlock;
			}
			else
			{
				freeBlocks = nextBlock;
			}
		}

		uintptr_t alignedAddress = (uintptr_t)currentBlock + adjustment;

		AllocationHeader* header = (AllocationHeader*)( alignedAddress - sizeof( AllocationHeader ) );
		header->size = totalSize;
		assert( alignment <= 0xffff && adjustment <= 0xffff ); //assert when alignment or adjustment does not fit in 16 bits.
		header->alignment = (uint16_t)alignment;
		header->adjustment = (uint16_t)adjustment;

		_UsedMemory += totalSize;
		_NumAllocations++;

		assert( ( alignedAddress & ( alignment - 1 ) ) == 0 );

		return (void*)alignedAddress;
	}

	return nullptr;
}

void FreeListAllocator::Deallocate( void* address )
{
	if ( address == nullptr )
	{
		return;
	}

	std::lock_guard<Mutex> guard( _Mutex );

	AllocationHeader* header = (AllocationHeader*)Util::AddPtr( address, -(int)sizeof( AllocationHeader ) );

	uintptr_t blockStart = reinterpret_cast<uintptr_t>( address ) - header->adjustment;
	size_t blockSize = header->size;
	uintptr_t blockEnd = blockStart + blockSize;

	FreeBlock* previousBlock = nullptr;
	FreeBlock* currentBlock = freeBlocks;

	while ( currentBlock != nullptr )
	{
		if ( (uintptr_t)currentBlock >= blockEnd )
		{
			break;
		}

		previousBlock = currentBlock;
		currentBlock = currentBlock->next;
	}

	if ( previousBlock == nullptr )
	{
		previousBlock = (FreeBlock*)blockStart;
		previousBlock->size = blockSize;
		previousBlock->next = freeBlocks;

		freeBlocks = previousBlock;
	}
	else if ( (uintptr_t)previousBlock + previousBlock->size == blockStart )
	{
		previousBlock->size += blockSize;
	}
	else
	{
		FreeBlock* temp = (FreeBlock*)blockStart;
		temp->size = blockSize;
		temp->next = previousBlock->next;
		previousBlock->next = temp;

		previousBlock = temp;
	}

	if ( currentBlock != nullptr && (uintptr_t)currentBlock == blockEnd )
	{
		previousBlock->size += currentBlock->size;
		previousBlock->next = currentBlock->next;
	}

	_NumAllocations--;
	_UsedMemory -= blockSize;
}

size_t FreeListAllocator::GetSizeOf( void* address )
{
	if ( address == nullptr )
	{
		return 0;
	}

	std::lock_guard<Mutex> guard( _Mutex );

	AllocationHeader* header = (AllocationHeader*)Util::AddPtr( address, -(int)sizeof( AllocationHeader ) );
	return header->size - header->adjustment;
}

size_t FreeListAllocator::GetAlignmentOf( void* address )
{
	if ( address == nullptr )
	{
		return 0;
	}

	std::lock_guard<Mutex> guard( _Mutex );

	AllocationHeader* header = (AllocationHeader*)Util::AddPtr( address, -(int)sizeof( AllocationHeader ) );
	return header->alignment;
}
