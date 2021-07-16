#pragma once
#include <vector>

template <typename T>
bool vectorsAreEqual(const std::vector<T> lhs, int skip, const std::vector<T> rhs)
{
	if (static_cast<int>(lhs.size()) <= skip)
	{
		return false;
	}

	if (lhs.size() - skip != rhs.size())
	{
		return false;
	}

	for (auto i = skip, j = 0; i < static_cast<int>(lhs.size()); ++i, ++j)
	{
		if (lhs[i] != rhs[j])
		{
			return false;
		}
	}
	return true;
}
