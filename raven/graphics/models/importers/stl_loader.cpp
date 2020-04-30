#include "stl_loader.h"

void STL::ReadBinary( const std::vector<char>& fileData, STLModel& STLData )
{
	const uint8_t* header = reinterpret_cast<const uint8_t*>( fileData.data() );

	if ( memcmp( header, "solid", sizeof( uint8_t ) * 5 ) == 0 )
	{
		ASSERT_FAIL( "This file is an ASCII STL file and not supported" );
		return;
	}

	memcpy( &STLData.header, fileData.data(), 80 );
	memcpy( &STLData.triangleCount, fileData.data() + 80, 4 );

	size_t bytesToRead = sizeof( STLTriangle ) * STLData.triangleCount;
	STLData.triangles = new STLTriangle[ STLData.triangleCount ];

	memcpy( STLData.triangles, fileData.data() + 84, bytesToRead );
}
