/*
 * ClientEventListener_CODE_GAME_POKER_PLAY.h
 *
 *  Created on: 2021年2月16日
 *      Author: san
 */

#ifndef LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_CODE_GAME_POKER_PLAY_H_
#define LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_CODE_GAME_POKER_PLAY_H_

#include "channel/ChannelUtils.h"
#include "ClientEventListener.h"
#include "muduo/net/TcpConnection.h"
#include <functional>
#include <iostream>

class ClientEventListener_CODE_GAME_POKER_PLAY : public ClientEventListener {
public:
	void operator()(const muduo::net::TcpConnectionPtr &conn, std::string data) override
	{
		std::string line;
		while (std::getline(std::cin, line))
		{
			std::transform(line.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写

			if (line == "")
			{
				printf("Invalid enter");
			}
			else
			{
				if ( line == "pass" || line == "p")
				{
					pushToServer(conn, ServerEventCode::CODE_GAME_POKER_PLAY_PASS, "");
				}
				else if (line == "exit" || line == "e")
				{
					pushToServer(conn, ServerEventCode::CODE_CLIENT_EXIT, "");
				}
				else
				{

				}
			}
		}
	}
};

#endif /* LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_CODE_GAME_POKER_PLAY_H_ */
