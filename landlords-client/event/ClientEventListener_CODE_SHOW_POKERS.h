/*
 * ClientEventListener_CODE_SHOW_POKERS.h
 *
 *  Created on: 2021年2月22日
 *      Author: san
 */

#ifndef LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_CODE_SHOW_POKERS_H_
#define LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_CODE_SHOW_POKERS_H_

#include "helper/SerializeHelper.h"
#include "helper/PokerHelper.h"

void show_pokers(const muduo::net::TcpConnectionPtr &conn, ClientEventCode code ,std::string data)
{
	ServerTransferData result = SerializeHelper::parseStringToData<ServerTransferData>(data);
	LOG_INFO << PokerHelper::printPokers(result.pokers_);
}

#endif /* LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_CODE_SHOW_POKERS_H_ */
