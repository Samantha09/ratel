/*
 * ServerEventListener.cc
 *
 *  Created on: 2021年2月22日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CC_
#define LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CC_

#include "ServerEventListener.h"
#include "EventFuncs.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

ServerEventListener::ServerEventListener()
	: LISTENER_MAP(std::unordered_map<ServerEventCode, SolveFunc>()),
	  dispatcher_(std::bind(&ServerEventListener::onUnknownMessage, this, _1, _2, _3)),
	  codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
{
	LISTENER_MAP[ServerEventCode::CODE_CLIENT_EXIT] = ServerEventListener_CODE_CLIENT_EXIT;
	LISTENER_MAP[ServerEventCode::CODE_CLIENT_NICKNAME_SET] = ServerEventListener_CODE_CLIENT_NICKNAME_SET;
	LISTENER_MAP[ServerEventCode::CODE_ROOM_CREATE_PVE] = ServerEventListener_CODE_ROOM_CREATE_PVE;
	LISTENER_MAP[ServerEventCode::CODE_GAME_STARTING] = ServerEventListener_CODE_GAME_STARTING;
	LISTENER_MAP[ServerEventCode::CODE_GAME_POKER_PLAY] = ServerEventListener_CODE_GAME_POKER_PLAY;
	LISTENER_MAP[ServerEventCode::CODE_GAME_POKER_PLAY_REDIRECT] = ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT;
	LISTENER_MAP[ServerEventCode::CODE_GAME_LANDLORD_ELECT] = ServerEventListener_CODE_GAME_LANDLORD_ELECT;
}


#endif /* LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CC_ */
