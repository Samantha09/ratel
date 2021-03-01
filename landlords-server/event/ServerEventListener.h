/*
 * ServerEventListener.h
 *
 *  Created on: 2021年2月15日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_H_
#define LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_H_

#include <unordered_map>
#include <algorithm>
#include "enums/ClientEventCode.h"
#include "enums/ServerEventCode.h"
#include "protobuf/codec.h"
#include "protobuf/dispatcher.h"
#include "protobuf/query.pb.h"

#include "muduo/base/Logging.h"

class ServerEventListener {
public:

	typedef void(*SolveFunc)( ProtobufCodec *codec,
			 const muduo::net::TcpConnectionPtr &conn,
			 ServerEventCode code, const std::string &data);

	ServerEventListener();
	// operator ()
	void operator() (const muduo::net::TcpConnectionPtr &conn,
			 ServerEventCode code, const std::string &data)
	{
		LOG_INFO << "ServerEventListener";
	    auto answerFuncIter = LISTENER_MAP.find(code);
	    if (answerFuncIter == LISTENER_MAP.end())
	    {
	    	LOG_INFO << "pushToClient";
	    	pushToClient(conn, ClientEventCode::CODE_CLIENT_EXIT, "unknown order");
	    	conn->shutdown();
	    }
	    SolveFunc answerFunc = answerFuncIter->second;
	    (*answerFunc)(&codec_, conn, code, data);
	}
	virtual ~ServerEventListener(){}

	void onUnknownMessage(const muduo::net::TcpConnectionPtr& conn,
	                      const MessagePtr& message,
	                      muduo::Timestamp)
	{
	  LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
	  conn->shutdown();
	}

	void pushToClient(const muduo::net::TcpConnectionPtr &conn,
			 ClientEventCode code, const char *data )
	{
		LOG_INFO << "pushToClient";
		muduo::Answer answer;
		answer.set_id(int(code));
		answer.set_answerer("san");
		answer.set_questioner("san");
		answer.add_solution(data);
		codec_.send(conn, answer);
	}

public:
	std::unordered_map<ServerEventCode, SolveFunc> LISTENER_MAP;
	ProtobufCodec codec_;
	ProtobufDispatcher dispatcher_;
};



#endif /* LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_H_ */
