/*
 * client.cc
 *
 *  Created on: 2021年2月9日
 *      Author: san
 */

#include "protobuf/dispatcher.h"
#include "protobuf/codec.h"
#include "protobuf/query.pb.h"
#include "event/ClientEventListener.h"
#include "helper/SerializeHelper.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpClient.h"
#include "muduo/net/EventLoopThread.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

typedef std::shared_ptr<muduo::Empty> EmptyPtr;
typedef std::shared_ptr<muduo::Answer> AnswerPtr;

google::protobuf::Message* messageToSend;


class QueryClient : noncopyable
{
 public:
  QueryClient(EventLoop* loop,
              const InetAddress& serverAddr)
  : loop_(loop),
    client_(loop, serverAddr, "QueryClient"),
    dispatcher_(std::bind(&QueryClient::onUnknownMessage, this, _1, _2, _3)),
    codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
  {
	dispatcher_.registerMessageCallback<muduo::Answer>(
		std::bind(&QueryClient::onAnswer, this, _1, _2, _3));
	dispatcher_.registerMessageCallback<muduo::Empty>(
		std::bind(&QueryClient::onEmpty, this, _1, _2, _3));
    client_.setConnectionCallback(
        std::bind(&QueryClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
  }

  void connect()
  {
    client_.connect();
  }

  TcpConnectionPtr getConnection()
  {
	  return client_.connection();
  }

  void onAnswer(const muduo::net::TcpConnectionPtr& conn,
                const AnswerPtr& message,
                muduo::Timestamp)
  {
//	  if (message->id() == int(ClientEventCode::CODE_CLIENT_CONNECT))
//	  {
//		  setId(atoi(message->solution(0).c_str()));
//	  }
//	  else

//	    MutexLockGuard lock(mutex_);
	  MapHelper result = SerializeHelper::parseStringToData<MapHelper>(message->solution(0));
	  clientEventListener(conn, id_, ClientEventCode(message->id()), result);

//    LOG_INFO << "onAnswer:\n" << message->GetTypeName() << message->DebugString();
  }

  void onEmpty(const muduo::net::TcpConnectionPtr&,
               const EmptyPtr& message,
               muduo::Timestamp)
  {
    LOG_INFO << "onEmpty: " << message->GetTypeName();
  }

//	void pushToServer(const muduo::net::TcpConnectionPtr &conn,
//					  ServerEventCode code,
//					  const std::string &data = "")
//	{
////		MutexLockGuard lock(mutex_)
//		ClientTransferData res(id_, code, data);
//		std::string result = SerializeHelper::SerializeToString<ClientTransferData>(res);
//		muduo::Query query;
//		query.set_id(int(cod
//		query.set_questioner("san");
//		query.add_question(result);
//	    codec_.send(conn, query);
//	}




 private:

  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_DEBUG << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
    	printf("Connected to server. Welcome to ratel!\n");

    }
    else
    {
      loop_->quit();
    }
  }

  void onUnknownMessage(const TcpConnectionPtr&,
                        const MessagePtr& message,
                        Timestamp)
  {
    LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
  }

  const int id_ = 21;                                 // room id
  EventLoop* loop_;
  TcpClient client_;
  ProtobufDispatcher dispatcher_;
  ProtobufCodec codec_;
  ClientEventListener clientEventListener;
  muduo::MutexLock mutex_;
};



int main(int argc, char* argv[])
{
	muduo::Logger::setLogLevel(muduo::Logger::DEBUG);
  LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
	EventLoop loop;
//	muduo::net::EventLoopThread loopThread;
	uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
	muduo::net::InetAddress serverAddr(argv[1], port);

	QueryClient client(&loop, serverAddr);
	client.connect();
	loop.loop();
//	std::string line;
//	while (std::getline(std::cin, line))
//	{
//		std::transform(line.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写
//
//		LOG_INFO << "please enter your order: ";
//		LOG_INFO << line;
//
//		if (line == "")
//		{
//			printf("Invalid enter");
//		}
//		else
//		{
//			if ( line == "pass" || line == "p")
//			{
//				client.pushToServer(client.getConnection(), ServerEventCode::CODE_GAME_POKER_PLAY_PASS, "");
//				sleep(0.5);
//			}
//			else if (line == "exit" || line == "e")
//			{
//				client.pushToServer(client.getConnection(), ServerEventCode::CODE_CLIENT_EXIT);
//			}
//			else if (line == "view" || line == "v")
//			{
////				client.pushToServer(client.getConnection(), code, data)
//			}
//		}
//
//	}
  }
  else
  {
	printf("Usage: %s host_ip port [q|e]\n", argv[0]);
  }
}



