/*
 * ClientEventListener.h
 *  监听客户端事件：不使用虚函数，采用std::function<> (NVI手法)
 *  Created on: 2021年2月11日
 *      Author: san
 */

#ifndef LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_H_
#define LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_H_

#include <unordered_map>
#include <functional>
#include "muduo/net/TcpConnection.h"
#include "entity/Poker.h"
#include "enums/ClientEventCode.h"
#include "enums/ServerEventCode.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"

#include "protobuf/dispatcher.h"
#include "protobuf/codec.h"
#include "protobuf/query.pb.h"
#include "EventFuns.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


class ClientEventListener {
public:

	typedef void(*settleEvent)(ProtobufCodec *codec,
				               const muduo::net::TcpConnectionPtr &conn,
							   int clientId,
							   const MapHelper &data);

	// 构造函数和析构函数
	// 构造函数
	ClientEventListener()
		:LISTENER_MAP(std::unordered_map<ClientEventCode, settleEvent>()),
		 dispatcher_(std::bind(&ClientEventListener::onUnknownMessage, this, _1, _2, _3)),
		 codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
	{
		LISTENER_MAP[ClientEventCode::CODE_SHOW_OPTIONS] = ClientEventListener_CODE_SHOW_OPTIONS;
		LISTENER_MAP[ClientEventCode::CODE_SHOW_POKERS] = ClientEventListener_CODE_SHOW_POKERS;
		LISTENER_MAP[ClientEventCode::CODE_GAME_POKER_PLAY_PASS] = ClientEventListener_CODE_GAME_POKER_PLAY_PASS;
		LISTENER_MAP[ClientEventCode::CODE_CLIENT_NICKNAME_SET] = ClientEventListener_CODE_CLIENT_NICKNAME_SET;
		LISTENER_MAP[ClientEventCode::CODE_GAME_STARTING] = ClientEventListener_CODE_GAME_STARTING;
		LISTENER_MAP[ClientEventCode::CODE_GAME_LANDLORD_CONFIRM] = ClientEventListener_CODE_GAME_LANDLOAD_CONFIRM;
		LISTENER_MAP[ClientEventCode::CODE_GAME_POKER_PLAY_REDIRECT] = ClientEventListener_CODE_GAME_POKER_PLAY_REDIRECT;
		LISTENER_MAP[ClientEventCode::CODE_GAME_POKER_PLAY] = ClientEventListener_CODE_GAME_POKER_PLAY;
		LISTENER_MAP[ClientEventCode::CODE_GAME_LANDLORD_ELECT] = ClientEventListener_CODE_GAME_LANDLOAD_ELECT;
		LISTENER_MAP[ClientEventCode::CODE_GAME_POKER_PLAY_ORDER_ERROR] = ClientEventListener_CODE_GAME_POKER_PLAY_ORDER_ERROR;
		LISTENER_MAP[ClientEventCode::CODE_GAME_OVER] = ClientEventListener_CODE_GAME_OVER;
		LISTENER_MAP[ClientEventCode::CODE_ROOM_CREATE_SUCCESS] = ClientEventListener_CODE_ROOM_CREATE_SUCCESS;
		LISTENER_MAP[ClientEventCode::CODE_ROOM_JOIN_SUCCESS] = ClientEventListener_CODE_ROOM_JOIN_SUCCESS;
		LISTENER_MAP[ClientEventCode::CODE_SHOW_ROOMS] = ClientEventListener_CODE_SHOW_ROOMS;
	}
	virtual ~ClientEventListener(){}

	void operator() (const muduo::net::TcpConnectionPtr &conn, int clientId,
					 ClientEventCode code ,const MapHelper &data)
	{

		auto answerFuncIter = LISTENER_MAP.find(code);
		if (answerFuncIter == LISTENER_MAP.end())
		{
			LOG_INFO << "CODE_CLIENT_EXIT " << int(code);
			pushDataToServer(&codec_, conn,
							 ServerEventCode::CODE_CLIENT_EXIT,
					         MapHelper());
		}
		else
		{
			answerFuncIter->second(&codec_, conn, clientId, data);
		}
	}

	// 根据code返回相应的函数对象
	void onUnknownMessage(const muduo::net::TcpConnectionPtr&,
						const MessagePtr& message,
						muduo::Timestamp)
	{
		LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
	}


protected:
//	static const std::vector<Poker> lastPokers;
//	static const std::string lastSellClientNickname;
//	static const std::string lastSellClientType;

//	static void initLastSellInfo()
//	{
//		lastPokers = NULL;
//		lastSellClientNickname = NULL;
//		lastSellClientType = NULL;
//	}

public:
	std::unordered_map<ClientEventCode, settleEvent> LISTENER_MAP;
	ProtobufCodec codec_;
	ProtobufDispatcher dispatcher_;
};


#endif /* LANDLORDS_CLIENT_EVENT_CLIENTEVENTLISTENER_H_ */
