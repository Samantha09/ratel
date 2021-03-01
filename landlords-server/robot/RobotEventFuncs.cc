/*
 * RobotEventFuncs.cc
 *
 *  Created on: 2021年2月26日
 *      Author: san
 */


#include "RobotEventFuns.h"
#include "../event/ServerContains.h"
#include "entity/Room.h"
#include "robot/RobotDecisionMakers.h"
#include "../event/EventFuncs.h"
#include <algorithm>
#include "muduo/base/Logging.h"

void RobotEventListener_CODE_GAME_LANDLORD_ELECT(ClientSide *robot,
												 ProtobufCodec *codec,
												 const muduo::net::TcpConnectionPtr &conn,
												 ServerEventCode code,
												 const std::string &data)
{
	LOG_INFO << "RobotEventListener_CODE_GAME_LANDLORD_ELECT";
	Room *room = ServerContains::getRoom(robot->getRoomId());
	std::vector<Poker> landlordPokers;
	std::copy(robot->getPokers().begin(),
			  robot->getPokers().end(),
			  std::back_inserter(landlordPokers));

	std::copy(room->getLoadlordPokers()->begin(),
			  room->getLoadlordPokers()->end(),
			  std::back_inserter(landlordPokers));

	std::vector<Poker> leftPokers = robot->getPre().getPokers();
	std::vector<Poker> rightPokers = robot->getNext().getPokers();

	PokerHelper::sortPoker(landlordPokers);
	PokerHelper::sortPoker(leftPokers);
	PokerHelper::sortPoker(rightPokers);

	ClientTransferData temp = SerializeHelper::parseStringToData<ClientTransferData>(data);
	bool res = RobotDecisionMakers::howToChooseLandlord(1, leftPokers, rightPokers, landlordPokers);
	std::string bres = res ? "true" : "false";
	temp.data = bres;
	std::string res1 = SerializeHelper::SerializeToString<ClientTransferData>(temp);
	ServerEventListener_CODE_GAME_LANDLORD_ELECT(codec, conn, code, res1);
}

void RobotEventListener_CODE_GAME_POKER_PLAY(ClientSide *robot,
											 ProtobufCodec *codec,
											 const muduo::net::TcpConnectionPtr &conn,
											 ServerEventCode code,
											 const std::string &data)
{
	LOG_DEBUG << "RobotEventListener_CODE_GAME_POKER_PLAY";
	Room *room = ServerContains::getRoom(robot->getRoomId());

	if (room->getLastSellClient() != robot->getId())
	{
		PokerSell lastPokerSell = room->getLastPokerShell();
		// FIXME: 在PokerSell中加入一个KONG
		PokerSell pokerSell = RobotDecisionMakers::howToPlayPokers(room->getDifficultyCoefficient(), lastPokerSell, *robot);

		// FIXME:
		if (true)
		{

		}

		if (pokerSell.getSellType() == SellType::ILLEGAL)
		{
			ServerEventListener_CODE_GAME_POKER_PLAY_PASS(codec, conn, code, "");
		}
		else
		{
			LOG_DEBUG << "pokerSell.getSellType()" << pokerSell.getSellType();
			ClientTransferData result(robot->getId(), ServerEventCode::CODE_GAME_POKER_PLAY, "");
			std::string res = SerializeHelper::SerializeToString<ClientTransferData>(result);
			ServerEventListener_CODE_GAME_POKER_PLAY(codec, conn, code, res);
		}
	}
}

