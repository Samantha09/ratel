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
												 const MapHelper &mapHelper)
{
	LOG_INFO << "RobotEventListener_CODE_GAME_LANDLORD_ELECT";
	Room *room = ServerContains::getRoom(robot->getRoomId());
	int turnClientId = mapHelper.get("nextClientId", 0);
	if (turnClientId == robot->getId())
	{
		LOG_INFO << "turnCLientID: " <<  turnClientId;
		LOG_INFO << "robot.id: " << robot->getId();
		sleep(3);
		ServerEventListener_CODE_GAME_LANDLORD_ELECT(codec, conn, MapHelper().put("clientId", robot->getId())
				 	 	 	 	 	 	 	 	 	 .put("is_Y", "false"));
	}
//	std::vector<Poker> landlordPokers;
//	std::copy(robot->getPokers().begin(),
//			  robot->getPokers().end(),
//			  std::back_inserter(landlordPokers));
//
//	std::copy(room->getLoadlordPokers()->begin(),
//			  room->getLoadlordPokers()->end(),
//			  std::back_inserter(landlordPokers));
//
//	std::vector<Poker> leftPokers = robot->getPre().getPokers();
//	std::vector<Poker> rightPokers = robot->getNext().getPokers();
//
//	PokerHelper::sortPoker(landlordPokers);
//	PokerHelper::sortPoker(leftPokers);
//	PokerHelper::sortPoker(rightPokers);

//	bool res = RobotDecisionMakers::howToChooseLandlord(1, leftPokers, rightPokers, landlordPokers);
//	std::string bres = res ? "true" : "false";
//	LOG_DEBUG << "is_Y: " << bres;
//	temp.data = bres;
//	std::string res1 = SerializeHelper::SerializeToString<ClientTransferData>(temp);

}

void RobotEventListener_CODE_GAME_POKER_PLAY(ClientSide *robot,
											 ProtobufCodec *codec,
											 const muduo::net::TcpConnectionPtr &conn,
											 const MapHelper &mapHelper)
{
	LOG_DEBUG << "机器人开始打牌";
	Room *room = ServerContains::getRoom(robot->getRoomId());
	PokerSell lastPokerSell = PokerSell(SellType::YAO_BU_QI, std::vector<Poker>(), 0);
	PokerSell pokerSell = PokerSell(SellType::YAO_BU_QI, std::vector<Poker>(), 0);
	LOG_DEBUG << "room->getLastSellClient(): " << room->getLastSellClient();
	LOG_DEBUG << "robot->getId(): " << robot->getId();
	if (room->getLastSellClient() != robot->getId())
	{
		lastPokerSell = room->getLastPokerShell();
		// FIXME: 在PokerSell中加入一个KONG
		PokerSell temp = RobotDecisionMakers::howToPlayPokers(room->getDifficultyCoefficient(), lastPokerSell, *robot);
	}
	else
	{
		pokerSell = RobotDecisionMakers::howToPlayPokers(room->getDifficultyCoefficient(), lastPokerSell, *robot);
	}

	if (pokerSell.getSellType() == SellType::ILLEGAL || pokerSell.getSellType() == SellType::YAO_BU_QI)
	{
		LOG_DEBUG << "机器人要不起！";
		LOG_DEBUG << "机器人的id： " << robot->getId();
		sleep(3);
		ServerEventListener_CODE_GAME_POKER_PLAY_PASS(codec, conn, MapHelper().put("clientId", robot->getId()));
	}
	else
	{

		LOG_DEBUG << "pokerSell.getSellType()" << pokerSell.getSellType();
//		ClientTransferData result(robot->getId(), ServerEventCode::CODE_GAME_POKER_PLAY, "");
//		std::string res = SerializeHelper::SerializeToString<ClientTransferData>(result);
		LOG_DEBUG << "机器人出了一张三";
		ServerEventListener_CODE_GAME_POKER_PLAY(codec, conn, MapHelper().put("clientId", robot->getId()).
																          put("options", std::vector<PokerLevel>({PokerLevel::LEVEL_3})));
	}

//		// FIXME:
//		if (true)
//		{
//
//		}
//	}
}

