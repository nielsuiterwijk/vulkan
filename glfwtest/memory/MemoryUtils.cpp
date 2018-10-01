#include "MemoryUtils.h"

size_t Util::Align(size_t size, size_t alignment, size_t& padding)
{
	padding = (alignment - (size % alignment)) % alignment;

	return size + padding;
}

size_t Util::Align(size_t size, size_t alignment)
{
	size_t padding;
	return Align(size, alignment, padding);
}

const void* Util::AlignPtr(const void* address, size_t alignment)
{
	uintptr_t ptr = reinterpret_cast<uintptr_t>(address);
	uintptr_t mask = static_cast<uintptr_t>(~(alignment - 1));

	return reinterpret_cast<const void*>(ptr & mask);
}

size_t Util::GetAlignedPtrAdjustment(const void* address, size_t alignment, size_t additionalOffset)
{
	uintptr_t ptr = reinterpret_cast<uintptr_t>(address);
	uintptr_t mask = static_cast<uintptr_t>(alignment - 1);
	size_t adjustment = alignment - (ptr & mask);

	if (adjustment == alignment)
	{
		adjustment = 0;
	}

	if (additionalOffset == 0)
	{
		return adjustment;
	}

	if (adjustment < additionalOffset)
	{
		additionalOffset -= adjustment;

		adjustment += alignment * (additionalOffset / alignment);

		if (additionalOffset % alignment > 0)
		{
			adjustment += alignment;
		}
	}

	return adjustment;
}

const void* Util::AddPtr(const void* address, int offset)
{
	uintptr_t ptr = reinterpret_cast<uintptr_t>(address);

	return reinterpret_cast<const void*>(ptr + offset);
}