/*
 * SimpleClient.cc
 *
 *  Created on: 2021年2月7日
 *      Author: san
 */

#include "SimpleClient.h"
#include "muduo/base/Logging.h"

using muduo::_1;
using muduo::_2;
using muduo::_3;

SimpleClient::SimpleClient(muduo::net::EventLoop *loop,
		 const muduo::net::InetAddress &serverAddr)
	: client_(loop, serverAddr, "SimpleClient"),
	  codec_(std::bind(&SimpleClient::onStringMessage, this, _1, _2, _3))
{
	client_.setConnectionCallback(
			std::bind(&SimpleClient::onConnection, this, _1));
	client_.setMessageCallback(
			std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
//	client_.enableRetry();
}

void SimpleClient::write(const std::string message)
{
	muduo::MutexLockGuard lock(mutex_);
	if (connection_)
	{
		codec_.send(muduo::get_pointer(connection_), message);
	}
}

void SimpleClient::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
	LOG_INFO << conn->localAddress().toIpPort() << " -> "
	             << conn->peerAddress().toIpPort() << " is "
	             << (conn->connected() ? "UP" : "DOWN");

	muduo::MutexLockGuard lock(mutex_);
	if (conn->connected())
	{
		connection_ = conn;
	}
	else
	{
		connection_.reset();
	}
}

void SimpleClient::onStringMessage(const muduo::net::TcpConnectionPtr &,
		             const std::string &message,
					 muduo::Timestamp)
{
	printf("%s\n", message.c_str());
}

SimpleClient::~SimpleClient() {
	// TODO Auto-generated destructor stub
}

