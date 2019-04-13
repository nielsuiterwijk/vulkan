#pragma once

#include "standard.h"

#pragma pack( push, 2 )
//https://en.wikipedia.org/wiki/STL_(file_format)#Binary_STL
struct STLTriangle
{
	glm::vec3 normal;
	glm::vec3 position[ 3 ];
	uint16_t attribute; //unused and should be '0'
};
#pragma pack( pop )

struct STLModel
{
	std::array<uint8_t, 80> header;
	STLTriangle* triangles;
	uint32_t triangleCount;
};

class STL
{
public:
	static void ReadBinary( const std::vector<char>& fileData, STLModel& STLData );
};
