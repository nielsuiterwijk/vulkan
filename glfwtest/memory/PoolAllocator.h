#pragma once

#include "Allocator.h"


class PoolAllocator: public Allocator
{
public:
	PoolAllocator();
	~PoolAllocator();

	void Initialize(size_t objectSize, size_t objectAlignment, size_t size, void* mem);

	void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override;
	void Deallocate(void* address) override;

	size_t GetSizeOf(void* address) override;
	size_t GetAlignmentOf(void* address) override;

	size_t GetObjectSize() const;
	size_t GetObjectAlignment() const;

private:
	PoolAllocator(const PoolAllocator&); // Prevent copies because it might cause errors
	PoolAllocator& operator=(const PoolAllocator&);

	size_t objectSize;
	size_t objectAlignment;

	void** freeList;

	static const char MemoryStatusOffset = sizeof(void*) + sizeof(int32_t);
};