#include "Allocator.h"

Allocator::Allocator() :
	start(nullptr),
	size(0),
	usedMemory(0),
	numAllocations(0)
{
}

Allocator::~Allocator()
{
	assert(numAllocations == 0 && usedMemory == 0);

	start = nullptr;
	size = 0;
}

void Allocator::Initialize(size_t size, void* start)
{
	assert(this->start == nullptr && "Already initialized allocator");
	this->start = start;
	this->size = size;
}