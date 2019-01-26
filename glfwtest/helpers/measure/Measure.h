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

std::chrono::high_resolution_clock::time_point measure_start, measure_pause;

template<typename F>
double measure(F f)
{
	using namespace std::chrono;

	static const int              num_trials = 6;
	static const milliseconds     min_time_per_trial(50);
	std::array<double, num_trials> trials;
	static decltype(f())          res; /* to avoid optimizing f() away */

	for (int i = 0; i<num_trials; ++i) 
	{
		int runs = 0;
		high_resolution_clock::time_point t2;

		measure_start = high_resolution_clock::now();

		do 
		{
			res = f();
			++runs;
			t2 = high_resolution_clock::now();
		} 
		while (t2 - measure_start < min_time_per_trial);

		trials[i] = duration_cast<duration<double>>(t2 - measure_start).count() / runs;
	}

	(void)res; /* var not used warn */

	std::sort(trials.begin(), trials.end());
	return std::accumulate(trials.begin() + 1, trials.end() - 1, 0.0) / (trials.size() - 2);
}

void pause_timing()
{
	measure_pause = std::chrono::high_resolution_clock::now();
}

void resume_timing()
{
	measure_start += std::chrono::high_resolution_clock::now() - measure_pause;
}