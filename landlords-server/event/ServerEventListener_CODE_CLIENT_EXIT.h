/*
 * ServerEventListener_CODE_CLIENT_EXIT.h
 *
 *  Created on: 2021年2月22日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CODE_CLIENT_EXIT_H_
#define LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CODE_CLIENT_EXIT_H_

# include "ServerEventListener.h"
#include "helper/SerializeHelper.h"


void client_exit(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code, const char *data)
{
	ClientTransferData res = SerializeHelper::parseStringToData<ClientTransferData>(data);
	LOG_ERROR << res.data;
	conn->shutdown();
}


#endif /* LANDLORDS_SERVER_EVENT_SERVEREVENTLISTENER_CODE_CLIENT_EXIT_H_ */
