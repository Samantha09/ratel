/*
 * ServerEventListener_CODE_GAME_POKER_PLAY.h
 *
 *  Created on: 2021年2月20日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CODE_GAME_POKER_PLAY_H_
#define LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CODE_GAME_POKER_PLAY_H_

#include "protobuf/codec.h"
#include "protobuf/dispatcher.h"
#include "protobuf/query.pb.h"

#include "enums/ServerEventCode.h"

void ServerEventListener_CODE_GAME_POKER_PLAY(const muduo::net::TcpConnectionPtr &conn,
		const ProtobufCodec &codec,
		ServerEventCode code, const char *data)
{

}




#endif /* LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CODE_GAME_POKER_PLAY_H_ */
