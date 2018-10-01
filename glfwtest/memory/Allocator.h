#pragma once

#include "standard.h"
#include "MemoryUtils.h"


class Allocator
{
public:
	Allocator();
	virtual ~Allocator();

	virtual void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) = 0;
	virtual void Deallocate(void* address) = 0;

	virtual size_t GetSizeOf(void* address) = 0;
	virtual size_t GetAlignmentOf(void* address) = 0;

	void* GetStart() const
	{
		return start;
	}

	void* GetEnd() const
	{
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(start) + size);
	}

	bool HasAddress(void* address)
	{
		return address >= GetStart() && address < GetEnd();
	}

	size_t GetSize() const
	{
		return size;
	}

	size_t GetTotalUsedMemory() const
	{
		return usedMemory;
	}

	size_t GetTotalFreeMemory() const
	{
		return size - usedMemory;
	}

	size_t GetNumAllocations() const
	{
		return numAllocations;
	}

protected:
	void Initialize(size_t size, void* start);

protected:
	void* start;
	size_t size;

	size_t usedMemory;
	size_t numAllocations;

	std::mutex lock;
};