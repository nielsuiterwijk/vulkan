#pragma once

#include "assert.h"

#if defined(_WIN32)
#define DEBUGBREAK_IMPL() __debugbreak();
#else
#define DEBUGBREAK_IMPL()
#endif

#define ASSERT_IMPL(x, msg)\
	do\
{\
	if (!(x)) \
	{\
	static bool ignore = false;\
	if (ignore == false)\
		{\
		int line = __LINE__;\
		if (Debug::HandleAssert(msg, line, __FILE__))\
			{\
			DEBUGBREAK_IMPL();\
			}\
			else\
			{\
			ignore=true;\
			}\
		}\
	}\
}\
	while (0)

#if NDEBUG

#define ASSERT(x) ((void)0)

#define ASSERT_MSG(x, msg) ((void)0)

#else

#if defined(_WIN32)

#define ASSERT(x) ASSERT_IMPL(x, "Assertion failed:\n\"" #x "\"")
#define ASSERT_MSG(x, msg) ASSERT_IMPL(x, "Assertion failed: \"" #x "\"\nMessage: " msg)

#else

#define ASSERT_IMPL2(x) do {if ((x) == false) { printf("%s(%d)\n", __FILE__, __LINE__); assert(x); }} while (0)
#define ASSERT(x) ASSERT_IMPL2(x)
#define ASSERT_MSG(x, msg) assert((x) && msg)

#endif

#endif

namespace Debug
{
	int HandleAssert(const char* msg, int line, const char* file);
}