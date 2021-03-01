/*
 * test4.cc
 *
 *  Created on: 2021年2月9日
 *      Author: san
 */

#include <stdint.h>
#include <iostream>
#include "muduo/net/Endian.h"

int main()
{
	int32_t be32 = 83886080;
	std::cout << be32toh(be32);
}

