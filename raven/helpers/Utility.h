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

	int32_t AvailableHardwareThreads();

}
