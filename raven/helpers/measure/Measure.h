#pragma once

/* 
* Copyright 2015 Joaquin M Lopez Munoz.
* Distributed under the Boost Software License, Version 1.0.
* (See accompanying file LICENSE_1_0.txt or copy at
* http://www.boost.org/LICENSE_1_0.txt)
*/

#include <algorithm>
#include <array>
#include <chrono>
#include <numeric>

namespace Measures
{
	namespace Details
	{
		struct SMeasureResult
		{
			double AverageTimePerCall = 0;
			int64_t NumberOfRuns = 0;
		};

		template <typename F>
		SMeasureResult Measure(F f, std::chrono::milliseconds TimePerTrial)
		{
			using namespace std::chrono;

			constexpr int NumTrials = 6;
			std::array<double, NumTrials> Trials;
			std::array<int64_t, NumTrials> Runs;
			static decltype(f()) TmpResult; /* to avoid optimizing f() away */

			high_resolution_clock::time_point Start;

			for (int i = 0; i < NumTrials; ++i)
			{
				int64_t Count = 0;
				high_resolution_clock::time_point t2;

				Start = high_resolution_clock::now();

				do
				{
					TmpResult = f();
					++Count;
					t2 = high_resolution_clock::now();
				} while (t2 - Start < TimePerTrial);

				duration<double> delta = t2 - Start;
				double deltaCount = duration_cast<duration<double>>(delta).count();
				Trials[i] = deltaCount / (double)Count;
				Runs[i] = Count;
			}

			(void)TmpResult; /* var not used warn */

			std::sort(Trials.begin(), Trials.end());
			std::sort(Runs.begin(), Runs.end());

			int64_t AverageRuns = std::accumulate(Runs.begin() + 1, Runs.end() - 1, 0LL);
			AverageRuns /= (Runs.size() - 2);

			double AverageTime = (double)std::accumulate(Trials.begin() + 1, Trials.end() - 1, 0.0) / (Trials.size() - 2);

			return { AverageTime, AverageRuns };
		}
	};

	template <typename F>
	Details::SMeasureResult MeasureShort(F f)
	{
		using namespace std::chrono;

		return Details::Measure(f, std::chrono::milliseconds(100));
	}

	template <typename F>
	Details::SMeasureResult MeasureLong(F f)
	{
		using namespace std::chrono;

		return Details::Measure(f, std::chrono::milliseconds(1000));
	}

	void Print(Details::SMeasureResult Result, const char* pLabel)
	{
		printf("%s: average time per run: %.9fs | %llu average runs \n", pLabel, Result.AverageTimePerCall, Result.NumberOfRuns);
	}
	

};