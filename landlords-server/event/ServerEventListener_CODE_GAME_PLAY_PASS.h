/*
 * ServerEventListener_CODE_GAME_PLAY_PASS.h
 *
 *  Created on: 2021年2月22日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CODE_GAME_PLAY_PASS_H_
#define LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CODE_GAME_PLAY_PASS_H_

//#include "ServerEventListener.h"
//#include "enums/PokerBasic.h"
//
////void pass( ProtobufCodec *codec,
////		const muduo::net::TcpConnectionPtr &conn,
////		ServerEventCode code, const char *data)
////{
////	std::vector<Poker> pokers{Poker(PokerType::HEART, PokerLevel::LEVEL_K)};
////	ServerTransferData res(pokers);
////	std::string result = SerializeHelper::SerializeToString<ServerTransferData>(res);
////	LOG_INFO << "CODE_GAME_PLAY_PASS";
////	muduo::Answer answer;
////	answer.set_id(int(ClientEventCode::CODE_GAME_POKER_PLAY_PASS));
////	answer.add_solution(result);
////	answer.set_questioner("chensuo");
////	answer.set_answerer("san");
////	codec->send(conn, answer);
////}
//
//#endif /* LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CODE_GAME_PLAY_PASS_H_ */
