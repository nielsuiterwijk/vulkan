#pragma once

#include "standard.h"

#undef min
#undef max

class AABB
{
public:
	explicit AABB() :
		min( 100, 100, 100 ),
		max( -100, -100, -100 ) {

		};

	~AABB() {};

	AABB( const AABB& mE ) = default;
	AABB( AABB&& mE ) = default;
	AABB& operator=( const AABB& mE ) = default;
	AABB& operator=( AABB&& mE ) = default;

	void Grow( const glm::vec3& newPoint )
	{
		max = glm::max( newPoint, max );
		min = glm::min( newPoint, min );
	}

	bool Overlaps( const AABB& other )
	{
		if ( min.x > other.max.x )
			return false;
		if ( min.y > other.max.y )
			return false;
		if ( min.z > other.max.z )
			return false;
		if ( max.x < other.min.x )
			return false;
		if ( max.y < other.min.y )
			return false;
		if ( max.z < other.min.z )
			return false;

		return true;

		//if (std::abs(centre.x - other.centre.x) > (extents.x + other.extents.x)) return false;
		//if (std::abs(centre.y - other.centre.y) > (extents.y + other.extents.y)) return false;
		//if (std::abs(centre.z - other.centre.z) > (extents.z + other.extents.z)) return false;
		//
		//return false;
	}

private:
	glm::vec3 min;
	glm::vec3 max;
};
