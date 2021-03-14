/*
 * RobotEventFuns.h
 *
 *  Created on: 2021年2月26日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_ROBOT_ROBOTEVENTFUNS_H_
#define LANDLORDS_SERVER_ROBOT_ROBOTEVENTFUNS_H_

#include "entity/ClientSide.h"
#include "enums/ServerEventCode.h"
#include "helper/SerializeHelper.h"

void RobotEventListener_CODE_GAME_LANDLORD_ELECT(ClientSide *robot,
												 ProtobufCodec *codec,
												 const muduo::net::TcpConnectionPtr &conn,
												 const MapHelper &data);

void RobotEventListener_CODE_GAME_POKER_PLAY(ClientSide *robot,
		 	 	 	 	 	 	 	 	 	 ProtobufCodec *codec,
											 const muduo::net::TcpConnectionPtr &conn,
											 const MapHelper &data);

#endif /* LANDLORDS_SERVER_ROBOT_ROBOTEVENTFUNS_H_ */
