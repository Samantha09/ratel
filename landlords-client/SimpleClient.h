/*
 * SimpleClient.h
 *
 *  Created on: 2021年2月7日
 *      Author: san
 */

#ifndef LANDLORDS_CLIENT_SIMPLECLIENT_H_
#define LANDLORDS_CLIENT_SIMPLECLIENT_H_

#include <string>
#include <vector>

#include "codec.h"

#include "muduo/net/protobuf/ProtobufCodecLite.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/base/Mutex.h"

class SimpleClient {
public:

//	static int id;
//	static std::string serverAddress;
//	static int port;

	SimpleClient(muduo::net::EventLoop *loop,
				 const muduo::net::InetAddress &serverAddr);
	~SimpleClient();

	void connect()
	{
		client_.connect();
	}

	void disconnect()
	{
		client_.disconnect();
	}

	void write(const std::string message);

private:
	void onConnection(const muduo::net::TcpConnectionPtr &conn);

	void onStringMessage(const muduo::net::TcpConnectionPtr &,
			             const std::string &message,
						 muduo::Timestamp);

private:
	static std::vector<std::string> serverAddressSource;
	static std::vector<std::string> getServerAddressList();

	muduo::net::TcpClient client_;
	muduo::net::TcpConnectionPtr connection_ GUARDED_BY(mutex_);
	muduo::MutexLock mutex_;
	LengthHeaderCodec codec_;
};

#endif /* LANDLORDS_CLIENT_SIMPLECLIENT_H_ */
