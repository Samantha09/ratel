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

std::vector<PokerLevel> parsePokerToLevel(const std::vector<Poker> &pokers);

void RobotEventListener_CODE_GAME_LANDLORD_ELECT(ClientSide *robot,
												 ProtobufCodec *codec,
												 const muduo::net::TcpConnectionPtr &conn,
												 const MapHelper &mapHelper)
{
	LOG_INFO << "RobotEventListener_CODE_GAME_LANDLORD_ELECT";
	std::shared_ptr<Room> room = ServerContains::getRoom(robot->getRoomId());
	int turnClientId = mapHelper.get("nextClientId", 0);
	if (turnClientId == robot->getId())
	{
		LOG_INFO << "turnCLientID: " <<  turnClientId;
		LOG_INFO << "robot.id: " << robot->getId();
		sleep(3);
		ServerEventListener_CODE_GAME_LANDLORD_ELECT(codec, conn, MapHelper().put("clientId", robot->getId())
				 	 	 	 	 	 	 	 	 	 .put("is_Y", "false"));
	}
}

void RobotEventListener_CODE_GAME_POKER_PLAY(ClientSide *robot,
											 ProtobufCodec *codec,
											 const muduo::net::TcpConnectionPtr &conn,
											 const MapHelper &mapHelper)
{
	LOG_DEBUG << "机器人开始打牌";
	std::shared_ptr<Room> room = ServerContains::getRoom(robot->getRoomId());
	PokerSell lastPokerSell;
	PokerSell pokerSell;
	std::vector<PokerSell> pokerSells;
	LOG_DEBUG << "room->getLastSellClient(): " << room->getLastSellClient();
	LOG_DEBUG << "robot->getId(): " << robot->getId();
	if (room->getLastSellClient() != robot->getId())
	{
		lastPokerSell = room->getLastPokerShell();
		LOG_DEBUG << "robot的手牌：" << PokerHelper::printPokers(robot->getPokers());
		LOG_DEBUG << "上家打的牌： " << PokerHelper::printPokers(*lastPokerSell.getSellPokers());
		lastPokerSell = PokerHelper::checkPokerType(*lastPokerSell.getSellPokers());
		pokerSells = PokerHelper::validSells(lastPokerSell, robot->getPokers());
		if (pokerSells.size() != 0)
			pokerSell = pokerSells.at(rand() % pokerSells.size());
	}
	else
	{
		pokerSells = PokerHelper::validSells(lastPokerSell, robot->getPokers());
		if (pokerSells.size() != 0)
			pokerSell = pokerSells.at(rand() % pokerSells.size());
	}

	if (pokerSell.getSellType() == SellType::ILLEGAL || pokerSell.getSellType() == SellType::VOID_SELL)
	{
		LOG_DEBUG << "机器人要不起！";
		LOG_DEBUG << "机器人的id： " << robot->getId();
		sleep(1);
		ServerEventListener_CODE_GAME_POKER_PLAY_PASS(codec, conn, MapHelper().put("clientId", robot->getId()));
	}
	else
	{
		LOG_DEBUG << "pokerSell.getSellType()" << pokerSell.getSellType();
		LOG_DEBUG << "机器人要的起";
		sleep(1);
		ServerEventListener_CODE_GAME_POKER_PLAY(codec, conn, MapHelper().put("clientId", robot->getId()).
																          put("options", parsePokerToLevel(*pokerSell.getSellPokers())));
	}

//		// FIXME:
//		if (true)
//		{
//
//		}
//	}
}


std::vector<PokerLevel> parsePokerToLevel(const std::vector<Poker> &pokers)
{
	std::vector<PokerLevel> result;
	for (const auto &p: pokers)
	{
		result.push_back(PokerLevel(p.getLevel()));
	}

	return result;
}

