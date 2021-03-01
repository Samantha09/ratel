/*
 * test1.cc
 *
 *  Created on: 2021年2月9日
 *      Author: san
 */

#include <stdint.h>
#include <iostream>
#include <cstring>

int main()
{
	int32_t be = 0;
	const char * s = "hello world";
	::memcpy(&be, s, 4);
	printf("%d", be);
}
