/*
 * test_set_vector.cc
 *
 *  Created on: 2021年2月20日
 *      Author: san
 */


#include <vector>
#include <unordered_set>
#include <iostream>
#include <algorithm>

struct MyKeyHashHasher
{
	size_t operator()(const std::vector<int> &k) const noexcept
	{
		return std::hash<const std::vector<int>*>{}(&k);
	}
};

struct MyKeyHashComparator
{
	bool operator()(const std::vector<int> &k1, const std::vector<int> &k2) const noexcept
	{
		return &k1 == &k2;
	}
};

int main()
{
	std::vector<int> ivec{0,1,2,3,4};
	std::unordered_set<std::vector<int>*> vecSet;
	vecSet.insert(&ivec);
	ivec.push_back(90);
	vecSet.insert(&ivec);
	vecSet.insert(&ivec);
	for (auto ps: vecSet)
	{
		std::for_each((*ps).begin(), (*ps).end(), [](int x){std::cout << x << " ";});
	}

	std::cout << std::endl;
	std::cout << vecSet.size();
}
