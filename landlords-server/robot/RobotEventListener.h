/*
 * RobotEventListener.h
 *
 *  Created on: 2021年2月26日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_ROBOT_ROBOTEVENTLISTENER_H_
#define LANDLORDS_SERVER_ROBOT_ROBOTEVENTLISTENER_H_

#include <unordered_map>
#include "enums/ClientEventCode.h"
#include "entity/ClientSide.h"
#include "enums/ServerEventCode.h"
#include "RobotEventFuns.h"
#include "muduo/base/Logging.h"

class RobotEventListener
{
public:
	typedef void(*RobotEventFunc)(
				 ClientSide *robot,
			 	 ProtobufCodec *codec,
				 const muduo::net::TcpConnectionPtr &conn,
				 const MapHelper &mapHelper);

	RobotEventListener(){}
	~RobotEventListener(){}

	static void init()
	{
		LISTENER_MAP.insert(std::make_pair(ClientEventCode::CODE_GAME_LANDLORD_ELECT, RobotEventListener_CODE_GAME_LANDLORD_ELECT));
		LISTENER_MAP.insert(std::make_pair(ClientEventCode::CODE_GAME_POKER_PLAY, RobotEventListener_CODE_GAME_POKER_PLAY));
	}

	static std::unordered_map<ClientEventCode, RobotEventFunc> LISTENER_MAP;

	static void get(ProtobufCodec *codec,
					const muduo::net::TcpConnectionPtr &conn,
					ClientEventCode code,
					ClientSide *robot,
					const MapHelper &mapHelper)
	{
		LOG_INFO << "RobotEventListener::get()";
		LOG_INFO << int(code);
		auto is_find = LISTENER_MAP.find(code);
		if (is_find != LISTENER_MAP.end())
		{
			is_find->second(robot, codec, conn,
							mapHelper);
		}
	}
};



#endif /* LANDLORDS_SERVER_ROBOT_ROBOTEVENTLISTENER_H_ */
