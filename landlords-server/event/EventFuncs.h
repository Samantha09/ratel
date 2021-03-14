/*
 * EventFuncs.h
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_EVENT_EVENTFUNCS_H_
#define LANDLORDS_SERVER_EVENT_EVENTFUNCS_H_

#include "protobuf/codec.h"
#include "helper/SerializeHelper.h"
#include "enums/ClientEventCode.h"

void ServerEventListener_CODE_CLIENT_EXIT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
										  const MapHelper &data);
void ServerEventListener_CODE_CLIENT_NICKNAME_SET(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											      const MapHelper &data);
void ServerEventListener_CODE_GAME_LANDLORD_ELECT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												  const MapHelper &data);
void ServerEventListener_CODE_GAME_POKER_PLAY_PASS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												   const MapHelper &data);
void ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
													   const MapHelper &data);
void ServerEventListener_CODE_GAME_POKER_PLAY(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  const MapHelper &data);
void ServerEventListener_CODE_GAME_STARTING(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											const MapHelper &data);
//void ServerEventListener_CODE_GAME_WATCH(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
//		const MapHelper &data);
void ServerEventListener_CODE_GET_ROOMS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
										const MapHelper &data);
void ServerEventListener_CODE_ROOM_CREATE_PVE(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  const MapHelper &data);
void ServerEventListener_CODE_ROOM_CREATE(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
										  const MapHelper &data);
void ServerEventListener_CODE_ROOM_JOIN(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
										const MapHelper &data);

void pushDataToClient(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
					  ClientEventCode code, const MapHelper &mapHelper);

#endif /* LANDLORDS_SERVER_EVENT_EVENTFUNCS_H_ */
