/*
 * test_distribute.cc
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */



#include "PokerHelper.h"
#include <iostream>

int main()
{
	PokerHelper ph;
	PokerHelper::init();
	std::vector<std::vector<Poker> > temp = PokerHelper::distributePoker();
	for (auto pokers: temp)
	{
		std::cout << PokerHelper::printPokers(pokers) << std::endl;
	}
}
