/*
 * test_checkPokerType.cc
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */

#include "PokerHelper.h"
#include "PokerBasic.h"
#include <iostream>

int main()
{
	std::vector<Poker> pokers;
  	Poker poker(PokerBasic::HEART, PokerBasic::LEVEL_K);
  	Poker poker1(PokerBasic::DIAMOND, PokerBasic::LEVEL_K);
  	Poker poker2(PokerBasic::CLUB, PokerBasic::LEVEL_K);
  	Poker poker3(PokerBasic::HEART, PokerBasic::LEVEL_4);
  	pokers.push_back(poker);
  	pokers.push_back(poker1);
  	pokers.push_back(poker2);
  	pokers.push_back(poker3);

  	PokerSell sell = PokerHelper::checkPokerType(pokers);
  	std::cout << "sell.pokers: " << std::endl;
  	std::string res = PokerHelper::printPokers(*(sell.getSellPokers()));
  	std::cout << res << std::endl;
  	std::cout << "sell.type: " << std::endl;
  	std::cout << sell.getSellType() << std::endl;
}
