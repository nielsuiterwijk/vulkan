#pragma once
#include <string>
#include <sstream>
#include <array>

namespace Helpers
{


	template <typename T>
	static std::string ValueToString(T value)
	{
		std::string val;

		std::stringstream ss;
		ss << value;

		return ss.str();
	}

	static std::string FormatWithCommas(int64_t value)
	{
		std::string numWithCommas = std::to_string(value);
		int insertPosition = (int)numWithCommas.length() - 3;

		while (insertPosition > 0)
		{
			numWithCommas.insert(insertPosition, ".");
			insertPosition -= 3;
		}

		return numWithCommas;
	}

	static double RoundToTwoDecimals(double n) 
	{
		double d = n * 100.0;
		int i = d + 0.5;
		d = (float)i / 100.0;
		return d;
	}
	
	static std::string FormatToSize(int64_t value)
	{		
		static std::array<std::string, 4> sizes = { std::string("B"), std::string("Kb"), std::string("Mb"), std::string("Gb") };		

		int div = 0;
		size_t rem = 0;

		while (value >= 1024 && div < sizes.size() ) {
			rem = (value % 1024);
			div++;
			value /= 1024;
		}

		double size_d = (float)value + (float)rem / 1024.0;
		return ValueToString(RoundToTwoDecimals(size_d)) + " " + sizes[div];
	}

}