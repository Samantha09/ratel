/*
 * main.cc
 *
 *  Created on: 2021年2月8日
 *      Author: san
 */


#include "SimpleClient.h"
#include "muduo/net/EventLoop.h"
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		muduo::net::EventLoop loop;
		uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
		muduo::net::InetAddress serverAddr(argv[1], port);

		SimpleClient client(&loop, serverAddr);
		client.connect();
		loop.loop();
//		std::string line;
//		while (std::getline(std::cin, line))
//		{
//			client.write(line);
//		}
		client.disconnect();
	}
	else
	{
		printf("Usage: %s host_ip port\n", argv[0]);
	}

}
