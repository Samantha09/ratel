/*
 * ClientEventListener_CODE_GAME_POKER_PLAY_CANT_PASS.h
 *
 *  Created on: 2021年2月23日
 *      Author: san
 */

#ifndef LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_CODE_GAME_POKER_PLAY_PASS_H_
#define LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_CODE_GAME_POKER_PLAY_PASS_H_

#include "ClientEventListener.h"
#include "helper/SerializeHelper.h"
#include "helper/PokerHelper.h"

#include "ClientEventListener_CODE_SHOW_POKERS.h"

void pass(const muduo::net::TcpConnectionPtr &conn, ClientEventCode code ,std::string data)
{
//	ServerTransferData res = SerializeHelper::parseStringToData<ServerTransferData>(data);
	LOG_INFO << "pass and show pokers";
	show_pokers(conn, code, data);
}


#endif /* LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_CODE_GAME_POKER_PLAY_PASS_H_ */
