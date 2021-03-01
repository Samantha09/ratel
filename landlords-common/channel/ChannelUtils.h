/*
 * ChannelUtils.h
 *
 *  Created on: 2021年2月12日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_UTILS_CHANNELUTILS_H_
#define LANDLORDS_COMMON_UTILS_CHANNELUTILS_H_

#include <string>
#include "enums/ClientEventCode.h"
#include "enums/ServerEventCode.h"
#include "protobuf/query.pb.h"
#include "protobuf/codec.h"

#include "muduo/net/TcpConnection.h"
#include "protobuf/codec.h"



//void pushToClient(const muduo::net::TcpConnectionPtr &conn,
//				  ProtobufCodec codec,
//				  ClientEventCode code,
//				  const std::string &data)
//{
//	// 将任务交给客户端代码去处理
//	muduo::Answer answer;
//	answer.set_id(code);
//	answer.set_answerer("san");
//	answer.add_solution(data);
//	codec.send(conn, answer);
//}
//
//void pushToServer(const muduo::net::TcpConnectionPtr &conn,
//		          ServerEventCode code,
//				  const std::string &data = "")
//{
//	// 将任务交给服务端代码去处理
//}

#endif /* LANDLORDS_COMMON_UTILS_CHANNELUTILS_H_ */
