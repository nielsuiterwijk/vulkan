#include "Allocator.h"

Allocator::Allocator() :
	start( nullptr ),
	size( 0 ),
	_UsedMemory( 0 ),
	_NumAllocations( 0 )
{
}

Allocator::~Allocator()
{
	assert( _NumAllocations == 0 && _UsedMemory == 0 );

	start = nullptr;
	size = 0;
}

void Allocator::Initialize( size_t size, void* start )
{
	assert( this->start == nullptr && "Already initialized allocator" );
	this->start = start;
	this->size = size;
}
