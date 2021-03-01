/*
 * test2.cc
 *
 *  Created on: 2021年2月9日
 *      Author: san
 */

#include <vector>
#include <iostream>
#include <endian.h>

int main()
{
	std::vector<char> buffer_{'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
	auto result = &*buffer_.begin() + 5;
	std::cout << result;

	std::cout << "===========================" << std::endl;
	const void *data = "♥";
	int32_t be32 = *static_cast<const int32_t*>(data);
	std::cout << be32 << std::endl;
	const int32_t len = be32toh(be32);
	std::cout << len << std::endl;

	return 0;
}


