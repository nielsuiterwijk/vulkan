#pragma once

#include "standard.h"

class Util
{
public:
	static size_t Align( size_t size, size_t alignment, size_t& padding );
	static size_t Align( size_t size, size_t alignment );

	static const void* AlignPtr( const void* address, size_t alignment );
	static size_t GetAlignedPtrAdjustment( const void* address, size_t alignment, size_t additionalOffset = 0 );
	static const void* AddPtr( const void* address, int offset );
};
