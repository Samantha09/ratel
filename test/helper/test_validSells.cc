/*
 * 测试正常
 */



#include "entity/Poker.h"
#include "entity/PokerSell.h"
#include "helper/PokerHelper.h"

#include "muduo/base/Logging.h"

#include <iostream>


int main()
{
	std::vector<Poker> pokers, lastPokers;
	lastPokers.push_back(Poker(PokerType::HEART, PokerLevel::LEVEL_3));
	lastPokers.push_back(Poker(PokerType::SPADE, PokerLevel::LEVEL_3));

	std::cout << "******************parseScore*******************" << std::endl;

	std::cout << "scores: " << PokerHelper::parseScore(SellType::DOUBLE, 0) << std::endl;

	pokers.push_back(Poker(PokerType::HEART, PokerLevel::LEVEL_4));
	pokers.push_back(Poker(PokerType::DIAMOND, PokerLevel::LEVEL_4));
	pokers.push_back(Poker(PokerType::SPADE, PokerLevel::LEVEL_4));
	pokers.push_back(Poker(PokerType::CLUB, PokerLevel::LEVEL_4));
	pokers.push_back(Poker(PokerType::CLUB, PokerLevel::LEVEL_5));
	pokers.push_back(Poker(PokerType::DIAMOND, PokerLevel::LEVEL_5));
	pokers.push_back(Poker(PokerType::CLUB, PokerLevel::LEVEL_5));
	pokers.push_back(Poker(PokerType::DIAMOND, PokerLevel::LEVEL_5));
	pokers.push_back(Poker(PokerType::DIAMOND, PokerLevel::LEVEL_6));
	pokers.push_back(Poker(PokerType::HEART, PokerLevel::LEVEL_6));
	pokers.push_back(Poker(PokerType::HEART, PokerLevel::LEVEL_7));

	PokerSell lastPokerSell(SellType::DOUBLE, std::vector<Poker>(), 1);
	lastPokerSell.setSellPokers(lastPokers);
	std::vector<PokerSell> result = PokerHelper::validSells(lastPokerSell, pokers);

	std::cout << "score: " << lastPokerSell.getScore() << std::endl;

	for (auto & p: result)
	{
		std::cout << p.getScore() << " " << PokerHelper::printPokers(*p.getSellPokers()) << std::endl;
	}
}
