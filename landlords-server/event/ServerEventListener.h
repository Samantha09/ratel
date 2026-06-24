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
#include "web/WsCodec.h"
#include "helper/SerializeHelper.h"

#include "muduo/base/Logging.h"

class ServerEventListener {
public:

	typedef void(*SolveFunc)( WsCodec *codec,
				 	 	 	 	  const muduo::net::TcpConnectionPtr &conn,
								  const MapHelper &mapHelper);

	ServerEventListener();
	// operator ()
	void operator() (const muduo::net::TcpConnectionPtr &conn,
				 ServerEventCode code, const MapHelper &mapHelper)
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
	    (*answerFunc)(&codec_, conn, mapHelper);
	}
	virtual ~ServerEventListener(){}

	void pushToClient(const muduo::net::TcpConnectionPtr &conn,
				 ClientEventCode code, const char * /*data*/ )
	{
		LOG_INFO << "pushToClient";
		codec_.sendEvent(conn, code, MapHelper());
	}

public:
	std::unordered_map<ServerEventCode, SolveFunc> LISTENER_MAP;
	WsCodec codec_;
};



#endif /* LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_H_ */
