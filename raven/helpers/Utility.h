#pragma once

#include <vector>

namespace Utility
{

	template <class T, class UnaryPredicate>
	const T& FindIf( const std::vector<T>& vector, UnaryPredicate filter )
	{
		auto iterator = std::find_if( vector.begin(), vector.end(), filter );
		ASSERT( iterator != vector.end() );
		return ( *iterator );
	}

	template <class T>
	T PopBack( std::vector<T>& vector )
	{
		size_t End = vector.size() - 1;
		T Last = vector[ End ];
		vector.erase( vector.begin() + End );
		return Last;
	}

	int32_t AvailableHardwareThreads();

#ifdef _MSC_VER
#include <intrin.h>

	uint32_t __inline ctz( uint32_t value )
	{
		DWORD trailing_zero = 0;

		if ( _BitScanForward( &trailing_zero, value ) )
		{
			return trailing_zero;
		}
		else
		{
			// This is undefined, I better choose 32 than 0
			return 32;
		}
	}

	uint32_t __inline clz( uint32_t value )
	{
		DWORD leading_zero = 0;

		if ( _BitScanReverse( &leading_zero, value ) )
		{
			return 31 - leading_zero;
		}
		else
		{
			// Same remarks as above
			return 32;
		}
	}
#endif

}
