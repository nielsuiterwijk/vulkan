#include "DebugAssert.h"
#include <string>
#include "Windows.h"
#include "Helpers.h"

int Debug::HandleAssert(const char* msg, int line, const char* file)
{
	std::string lineStr = Helpers::ValueToString(line);

	std::string s = std::string(msg) + "\nFile: " + file + ":" + lineStr;

#if defined(_WIN32)
	switch (MessageBoxA(NULL, s.c_str(), "Assertion failed!", MB_ABORTRETRYIGNORE | MB_ICONWARNING))
	{
	case IDABORT:
		exit(1);
	case IDIGNORE:
		return 0;
		break;
	case IDRETRY:
	default:
		return 1;
		break;
	}
#endif
	return 0;
}