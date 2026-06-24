/*
 * EventFuncs.h
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_EVENT_EVENTFUNCS_H_
#define LANDLORDS_SERVER_EVENT_EVENTFUNCS_H_

#include "web/WsCodec.h"
#include "helper/SerializeHelper.h"
#include "enums/ClientEventCode.h"

void ServerEventListener_CODE_CLIENT_EXIT(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  const MapHelper &data);
void ServerEventListener_CODE_CLIENT_NICKNAME_SET(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												      const MapHelper &data);
void ServerEventListener_CODE_GAME_LANDLORD_ELECT(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
													  const MapHelper &data);
void ServerEventListener_CODE_GAME_POKER_PLAY_PASS(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
													   const MapHelper &data);
void ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
														   const MapHelper &data);
void ServerEventListener_CODE_GAME_POKER_PLAY(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												  const MapHelper &data);
void ServerEventListener_CODE_GAME_STARTING(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												const MapHelper &data);
//void ServerEventListener_CODE_GAME_WATCH(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
//		const MapHelper &data);
void ServerEventListener_CODE_GET_ROOMS(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											const MapHelper &data);
void ServerEventListener_CODE_ROOM_CREATE_PVE(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												  const MapHelper &data);
void ServerEventListener_CODE_ROOM_CREATE(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  const MapHelper &data);
void ServerEventListener_CODE_ROOM_JOIN(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											const MapHelper &data);

void pushDataToClient(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
						  ClientEventCode code, const MapHelper &mapHelper);

#endif /* LANDLORDS_SERVER_EVENT_EVENTFUNCS_H_ */
