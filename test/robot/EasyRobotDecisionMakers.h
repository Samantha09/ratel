/*
 * EasyRobotDecisionMakers.h
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ROBOT_EASYROBOTDECISIONMAKERS_H_
#define LANDLORDS_COMMON_ROBOT_EASYROBOTDECISIONMAKERS_H_

#include "helper/PokerHelper.h"
#include "enums/SellType.h"
#include "entity/ClientSide.h"

#include <cstdlib>

#define random(x) rand()%(x)

class EasyRobotDecisionMakers{

public:
	static PokerSell howToPlayPokers(PokerSell &lastPokerSell,
							  ClientSide &robot)
	{
		if (  // FIXME
		    lastPokerSell.getSellType() == SellType::KING_BOMB)
		{
			// FIXME
			return PokerSell(SellType::ILLEGAL, std::vector<Poker>(), 0);
		}

		std::vector<PokerSell> sells = PokerHelper::parsePokerSells(robot.getPokers());
		return sells.at(random(sells.size()));

		for (PokerSell &sell: sells)
		{
			if(sell.getSellType() == lastPokerSell.getSellType())
			{
				if(sell.getScore() > lastPokerSell.getScore() &&
				   sell.getSellPokers()->size() == lastPokerSell.getSellPokers()->size())
				{
					return sell;
				}
			}
		}

		if(lastPokerSell.getSellType() != SellType::BOMB)
		{
			for (PokerSell &sell: sells)
			{
				if (sell.getSellType() == SellType::BOMB)
				{
					return sell;
				}
			}
		}

		for(PokerSell sell: sells)
		{
			if(sell.getSellType() == SellType::KING_BOMB) {
				return sell;
			}
		}
		// FIXME
		return PokerSell(SellType::ILLEGAL, std::vector<Poker>(), 0);
	}

	static bool howToChooseLandlord(std::vector<Poker> leftPokers,
			 std::vector<Poker> rightPokers,
			 std::vector<Poker> myPokers)
	{
		std::vector<PokerSell> leftSells = PokerHelper::parsePokerSells(leftPokers);
		std::vector<PokerSell> mySells = PokerHelper::parsePokerSells(myPokers);
		std::vector<PokerSell> rightSells = PokerHelper::parsePokerSells(rightPokers);
		return mySells.size() > leftSells.size() && mySells.size() > rightSells.size();
	}
};




#endif /* LANDLORDS_COMMON_ROBOT_EASYROBOTDECISIONMAKERS_H_ */
