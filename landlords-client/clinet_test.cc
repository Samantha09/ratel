/*
 * clinet_test.cc
 *
 *  Created on: 2021年2月9日
 *      Author: san
 */

#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpClient.h"

#include "muduo/base/Logging.h"

#include <boost/bind.hpp>

#include <utility>

#include <stdio.h>
#include <unistd.h>

std::string message = "Hello\n";

void onConnection(const typename muduo::net::TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toIpPort().c_str());
    conn->send(message);
  }
  else
  {
    printf("\nonConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

void onMessage(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp receiveTime)
{
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime.toFormattedString().c_str());

  std::string message(buf->peek(), buf->peek() + buf->readableBytes());
  printf("onMessage(): \n%s", message.c_str());
}

int main()
{
  muduo::net::EventLoop loop;
  muduo::net::InetAddress serverAddr("localhost", 9981);
  muduo::net::TcpClient client(&loop, serverAddr, "client_test");

  client.setConnectionCallback(onConnection);
  client.setMessageCallback(onMessage);
//  client.enableRetry();
  client.connect();
  loop.loop();
}




