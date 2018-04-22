#pragma once

#pragma warning( push )
#pragma warning( disable: 4127 )

#include "DebugAssert.h"

typedef unsigned char byte;
typedef unsigned char uint8;
typedef char sbyte;

typedef short int16;
typedef unsigned short uint16;

typedef unsigned int uint32;
typedef int int32;

typedef unsigned long long uint64;
typedef long long int64;


class Integer
{
public:
	static void RunSanityCheck()
	{
		ASSERT(sizeof(byte) == 1);
		ASSERT(sizeof(uint8) == 1);
		ASSERT(sizeof(sbyte) == 1);

		ASSERT(sizeof(int16) == 2);
		ASSERT(sizeof(uint16) == 2);

		ASSERT(sizeof(uint32) == 4);
		ASSERT(sizeof(int32) == 4);

		ASSERT(sizeof(uint64) == 8);
		ASSERT(sizeof(int64) == 8);
	}

	static constexpr sbyte MAX_INT8 = 127;
	static constexpr int16 MAX_INT16 = 32767;
	static constexpr int32 MAX_INT32 = 2147483647;

	static constexpr sbyte MIN_INT8 = -127;
	static constexpr short MIN_INT16 = -32767;
	static constexpr int MIN_INT32 = -2147483647;

	static constexpr byte MAX_UINT8 = 255U;
	static constexpr uint16 MAX_UINT16 = 65535U;
	static constexpr uint32 MAX_UINT32 = 4294967295U;

	static constexpr int64 MIN_INT64 = -9223372036854775807;
	static constexpr int64 MAX_INT64 = 9223372036854775807;

	static constexpr uint64 MAX_UINT64 = 18446744073709551615U;
};

#pragma warning(pop)