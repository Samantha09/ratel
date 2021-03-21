/*
 * RobotDecisionMakers.h
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ROBOT_ROBOTDECISIONMAKERS_H_
#define LANDLORDS_COMMON_ROBOT_ROBOTDECISIONMAKERS_H_

#include <unordered_map>
#include "EasyRobotDecisionMakers.h"

class RobotDecisionMakers
{
public:
	typedef PokerSell(*PlayPokersFunc)(PokerSell &lastPokerSell,
			 	 	 	 	 	 	   ClientSide &robot);
	typedef bool(*ChooseLandlordFunc)(std::vector<Poker> leftPokers,
									  std::vector<Poker> rightPokers,
									  std::vector<Poker> myPokers);
	RobotDecisionMakers()
	{
	};
	static void init()
	{
		PlayPokersFuncMap.insert( std::make_pair(1,
						EasyRobotDecisionMakers::howToPlayPokers));
		ChooseLandlordFuncMap.insert(std::make_pair(1,
						EasyRobotDecisionMakers::howToChooseLandlord));
	}

	static bool contains(int difficultyCoefficient)
	{
		auto resIter = PlayPokersFuncMap.find(difficultyCoefficient);
		if (resIter != PlayPokersFuncMap.end())
			return true;
		return false;
	}

	static PokerSell howToPlayPokers(int difficultyCoefficient, PokerSell &lastPokerSell,
									 ClientSide &robot)
	{
		LOG_DEBUG << "howToPlayPokers";
		LOG_DEBUG << "difficultyCoefficient" << difficultyCoefficient;
		return PokerSell(SellType::VOID_SELL, std::vector<Poker>(), 0);
//		return PlayPokersFuncMap[difficultyCoefficient](*lastPokerSell, robot);
	}

	static bool howToChooseLandlord(int difficultyCoefficient,
			 	 	 	 	 	 	std::vector<Poker> leftPokers,
									std::vector<Poker> rightPokers,
									std::vector<Poker> myPokers)
	{
		LOG_INFO << "howToChooseLandlord";
		return ChooseLandlordFuncMap[difficultyCoefficient](leftPokers, rightPokers, myPokers);
	}

private:
	static std::unordered_map<int, PlayPokersFunc> PlayPokersFuncMap;
	static std::unordered_map<int, ChooseLandlordFunc> ChooseLandlordFuncMap;
};



#endif /* LANDLORDS_COMMON_ROBOT_ROBOTDECISIONMAKERS_H_ */
