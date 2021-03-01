/*
 * test3.cc
 *
 *  Created on: 2021年2月9日
 *      Author: san
 */

#include "muduo/net/Buffer.h"
#include "muduo/net/Endian.h"
#include <iostream>

int main()
{
	muduo::net::Buffer buf;
	buf.append("hello");

	std::cout << "buf.readableBytes(): " << buf.readableBytes() << std::endl;
	std::cout << "buf.peekInt32(): " << buf.peekInt32() << std::endl;

	const void *data = buf.peek();
	int32_t be32 = *static_cast<const int32_t*>(data);
	std::cout << be32toh(83886080);




}
