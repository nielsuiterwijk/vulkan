#pragma once

#pragma warning( push )
#pragma warning( disable: 4127 )

typedef unsigned char byte;
typedef unsigned char uint8;
typedef char sbyte;

typedef short int16;
typedef unsigned short ushort;
typedef unsigned short uint16;

typedef unsigned int uint;
typedef unsigned int uint32;
typedef int int32;

typedef unsigned long long uint64;


class Integer
{
public:
	static void RunSanityCheck()
	{
		EE_ASSERT(sizeof(byte) == 1);
		EE_ASSERT(sizeof(uint8) == 1);
		EE_ASSERT(sizeof(sbyte) == 1);

		EE_ASSERT(sizeof(int16) == 2);
		EE_ASSERT(sizeof(ushort) == 2);
		EE_ASSERT(sizeof(uint16) == 2);

		EE_ASSERT(sizeof(uint) == 4);
		EE_ASSERT(sizeof(uint32) == 4);
		EE_ASSERT(sizeof(int32) == 4);

		EE_ASSERT(sizeof(uint64) == 8);
	}

	static const sbyte MAX_INT8 = 127;
	static const short MAX_INT16 = 32767;
	static const int MAX_INT32 = 2147483647;

	static const sbyte MIN_INT8 = -128;
	static const short MIN_INT16 = -32767 - 1;
	static const int MIN_INT32 = -2147483647 - 1;

	static const byte MAX_UINT8 = 255;
	static const ushort MAX_UINT16 = 65535;
	static const uint MAX_UINT32 = 4294967295U;
};

#pragma warning(pop)