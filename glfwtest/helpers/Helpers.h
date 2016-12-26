#pragma once
#include <string>
#include <sstream>

namespace Helpers
{


	template <typename T>
	std::string ValueToString(T value)
	{
		std::string val;

		std::stringstream ss;
		ss << value;

		return ss.str();
	}

}