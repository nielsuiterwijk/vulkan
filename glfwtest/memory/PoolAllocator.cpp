#include "PoolAllocator.h"

PoolAllocator::PoolAllocator() :
	objectSize( 0 ),
	objectAlignment( 0 )
{
}

PoolAllocator::~PoolAllocator()
{
	freeList = nullptr;
}

void PoolAllocator::Initialize( size_t objectSize, size_t objectAlignment, size_t size, void* mem )
{
	Allocator::Initialize( size, mem );

	assert( objectSize >= MemoryStatusOffset );

	this->objectSize = objectSize;
	this->objectAlignment = objectAlignment;

	// Calculate adjustment needed to keep objects correctly aligned
	size_t adjustment = Util::GetAlignedPtrAdjustment( mem, objectAlignment );

	freeList = (void**)Util::AddPtr( mem, (int)adjustment );

	// when the alignment is bigger than the object size, adjust the object size so objects keep aligned
	if ( objectAlignment > objectSize )
	{
		this->objectSize = objectAlignment;
	}

	size_t numObjects = ( size - adjustment ) / this->objectSize;

	void** address = freeList;

	// Initialize free blocks list
	for ( size_t i = 0; i < numObjects - 1; i++ )
	{
		*address = (void*)Util::AddPtr( address, (int)this->objectSize );
		address = (void**)*address;
	}

	*address = nullptr;
}

void* PoolAllocator::Allocate( size_t size, size_t alignment )
{
	assert( size <= objectSize && alignment <= objectAlignment );

	std::lock_guard<std::mutex> guard( lock );

	if ( freeList == nullptr )
	{
		return nullptr;
	}

	void* address = freeList;

	freeList = (void**)( *freeList );

	assert( ( reinterpret_cast<uintptr_t>( address ) & ( objectAlignment - 1 ) ) == 0 );

	usedMemory += objectSize;
	numAllocations++;

	return address;
}

void PoolAllocator::Deallocate( void* address )
{
	std::lock_guard<std::mutex> guard( lock );

	*( (void**)address ) = freeList;
	freeList = (void**)address;

	usedMemory -= objectSize;
	numAllocations--;
}

size_t PoolAllocator::GetSizeOf( void* address )
{
	return objectSize;
}

size_t PoolAllocator::GetAlignmentOf( void* address )
{
	return objectAlignment;
}

size_t PoolAllocator::GetObjectSize() const
{
	return objectSize;
}

size_t PoolAllocator::GetObjectAlignment() const
{
	return objectAlignment;
}
