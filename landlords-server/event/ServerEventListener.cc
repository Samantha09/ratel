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

	ServerEventListener::ServerEventListener()
		: LISTENER_MAP(std::unordered_map<ServerEventCode, SolveFunc>()),
		  codec_([](const muduo::net::TcpConnectionPtr&, const std::string&) {
		      // Default JSON callback - does nothing, server.cc handles dispatch
		  })
	{
	LISTENER_MAP[ServerEventCode::CODE_CLIENT_EXIT] = ServerEventListener_CODE_CLIENT_EXIT;
	LISTENER_MAP[ServerEventCode::CODE_CLIENT_NICKNAME_SET] = ServerEventListener_CODE_CLIENT_NICKNAME_SET;
	LISTENER_MAP[ServerEventCode::CODE_ROOM_CREATE_PVE] = ServerEventListener_CODE_ROOM_CREATE_PVE;
	LISTENER_MAP[ServerEventCode::CODE_GAME_STARTING] = ServerEventListener_CODE_GAME_STARTING;
	LISTENER_MAP[ServerEventCode::CODE_GAME_POKER_PLAY] = ServerEventListener_CODE_GAME_POKER_PLAY;
	LISTENER_MAP[ServerEventCode::CODE_GAME_POKER_PLAY_REDIRECT] = ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT;
	LISTENER_MAP[ServerEventCode::CODE_GAME_LANDLORD_ELECT] = ServerEventListener_CODE_GAME_LANDLORD_ELECT;
	LISTENER_MAP[ServerEventCode::CODE_GAME_POKER_PLAY_PASS] = ServerEventListener_CODE_GAME_POKER_PLAY_PASS;
	LISTENER_MAP[ServerEventCode::CODE_ROOM_CREATE] = ServerEventListener_CODE_ROOM_CREATE;
	LISTENER_MAP[ServerEventCode::CODE_ROOM_JOIN] = ServerEventListener_CODE_ROOM_JOIN;
	LISTENER_MAP[ServerEventCode::CODE_GET_ROOMS] = ServerEventListener_CODE_GET_ROOMS;
}


#endif /* LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CC_ */
