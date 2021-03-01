/*
 * PokerSell.cc
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */


#include "helper/PokerHelper.h"
#include "PokerSell.h"

PokerSell::PokerSell(SellType sellType, std::vector<Poker> sellPokers, int coreLevel)
		: score_(PokerHelper::parseScore(sellType, coreLevel)),
		  sellType_(sellType),
		  sellPokers_(sellPokers),
		  coreLevel_(coreLevel)
{
}

