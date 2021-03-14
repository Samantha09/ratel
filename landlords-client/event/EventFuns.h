/*
 * EventFuns.h
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */

#ifndef LANDLORDS_CLIENT_EVENT_EVENTFUNS_H_
#define LANDLORDS_CLIENT_EVENT_EVENTFUNS_H_


#include "helper/SerializeHelper.h"
#include "helper/PokerHelper.h"
#include <stdio.h>

void ClientEventListener_CODE_CLIENT_EXIT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  int clientId, const MapHelper &data);
//void ClientEventListener_CODE_CLIENT_KICK(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
void ClientEventListener_CODE_CLIENT_NICKNAME_SET(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												  int clientId, const MapHelper &data);
//void ClientEventListener_CODE_GAME_LANDLOAD_SET(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
void ClientEventListener_CODE_GAME_LANDLOAD_CONFIRM(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  	  	  	int clientId, const MapHelper &data);
//void ClientEventListener_CODE_GAME_LANDLOAD_CYCLE(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
void ClientEventListener_CODE_GAME_LANDLOAD_ELECT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  	      int clientId, const MapHelper &data);
//void ClientEventListener_CODE_GAME_OVER(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//void ClientEventListener_CODE_GAME_POKER_PLAY_CANT_PASS(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
void ClientEventListener_CODE_GAME_POKER_PLAY_INVALID(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  	  	      int clientId, const MapHelper &data);
//void ClientEventListener_CODE_GAME_POKER_PLAY_LESS(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//void ClientEventListener_CODE_GAME_POKER_PLAY_MISMATCH(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
void ClientEventListener_CODE_GAME_POKER_PLAY_ORDER_ERROR(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  	  	  	  	  int clientId, const MapHelper &data);
void ClientEventListener_CODE_GAME_POKER_PLAY_PASS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  	  	   int clientId, const MapHelper &data);
void ClientEventListener_CODE_GAME_POKER_PLAY_REDIRECT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
													   int clientId, const MapHelper &data);
void ClientEventListener_CODE_GAME_POKER_PLAY(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  int clientId, const MapHelper &data);
void ClientEventListener_CODE_GAME_STARTING(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	    int clientId, const MapHelper &data);
//void ClientEventListener_CODE_GAME_WATCH_SUCCESSFUL(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//void ClientEventListener_CODE_GAME_WATCH(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//void ClientEventListener_CODE_PVE_DIFFICULT_NOT_SUPPORT(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//void ClientEventListener_CODE_ROOM_CREATE_SUCCESS(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//void ClientEventListener_CODE_ROOM_JOIN_FAIL_BY_FULL(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//void ClientEventListener_CODE_ROOM_JOIN_FAIL_BY_INEXIST(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//void ClientEventListener_CODE_ROOM_JOIN_SUCCESS(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//void ClientEventListener_CODE_ROOM_PLAY_FAIL_BY_INEXIST(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
void ClientEventListener_CODE_SHOW_OPTION_PVE(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  int clientId, const MapHelper &data);
void ClientEventListener_CODE_SHOW_OPTION_PVP(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  	  int clientId, const MapHelper &data);
void ClientEventListener_CODE_SHOW_OPTION_SETTING(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  	  int clientId, const MapHelper &data);
void ClientEventListener_CODE_SHOW_OPTIONS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	       int clientId, const MapHelper &data);
void ClientEventListener_CODE_SHOW_POKERS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  int clientId, const MapHelper &data);
//void ClientEventListener_CODE_SHOW_ROOMS(const muduo::net::TcpConnectionPtr &conn,
//		ClientEventCode code, const std::string &data);
//

void pushDataToServer(ProtobufCodec *codec,
					  const muduo::net::TcpConnectionPtr &conn,
					  ServerEventCode code,
					  const MapHelper &data);



#endif /* LANDLORDS_CLIENT_EVENT_EVENTFUNS_H_ */
