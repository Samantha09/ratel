/*
 * ServerEventListemer_CODE_GET_ROOMS.h
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_EVENT_SERVEREVENTLISTEMER_CODE_GET_ROOMS_H_
#define LANDLORDS_SERVER_EVENT_SERVEREVENTLISTEMER_CODE_GET_ROOMS_H_

#include "ServerEventListener.h"
#include "enums/PokerBasic.h"

void get_room(ProtobufCodec *codec,
		const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code, const char *data)
{

}



#endif /* LANDLORDS_SERVER_EVENT_SERVEREVENTLISTEMER_CODE_GET_ROOMS_H_ */
