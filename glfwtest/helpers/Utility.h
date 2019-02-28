#pragma once

template<class T, class UnaryPredicate>
const T& FindIf(const std::vector<T>& vector, UnaryPredicate filter)
{
	auto iterator = std::find_if(vector.begin(), vector.end(), filter);
	return (*iterator);
}
