#pragma once
#include <array>
#include <sstream>
#include <string>

namespace Helpers
{

template <typename T>
static std::string ValueToString( T value )
{
	std::string val;

	std::stringstream ss;
	ss << value;

	return ss.str();
}

static std::string FormatWithCommas( int64_t value )
{
	std::string numWithCommas = std::to_string( value );
	int insertPosition = (int)numWithCommas.length() - 3;

	while ( insertPosition > 0 )
	{
		numWithCommas.insert( insertPosition, "." );
		insertPosition -= 3;
	}

	return numWithCommas;
}

static double RoundToTwoDecimals( double n )
{
	double d = n * 100.0;
	int i = (int)( d + 0.5 );
	d = (float)i / 100.0;
	return d;
}

static std::string MemorySizeToString( int64_t value )
{
	static std::array<std::string, 4> sizes = { std::string( "B" ), std::string( "KB" ), std::string( "MB" ), std::string( "GB" ) };

	int div = 0;
	size_t rem = 0;

	while ( value >= 1024 && div < sizes.size() )
	{
		rem = ( value % 1024 );
		div++;
		value /= 1024;
	}

	double size_d = (float)value + (float)rem / 1024.0;
	return ValueToString( RoundToTwoDecimals( size_d ) ) + "" + sizes[ div ];
}

static void PadString( std::string& text, size_t num, char paddingChar = ' ' )
{
	if ( num > text.size() )
		text.insert( 0, num - text.size(), paddingChar );
}

static std::string GetFileExtension( const std::string& fileName )
{
	//source: https://stackoverflow.com/questions/51949/how-to-get-file-extension-from-string-in-c
	if ( fileName.find_last_of( "." ) != std::string::npos )
	{
		return fileName.substr( fileName.find_last_of( "." ) + 1 );
	}

	return "";
}

}
