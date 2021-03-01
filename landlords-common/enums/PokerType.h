/*
 * PokerType.h
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENUMS_POKERTYPE_H_
#define LANDLORDS_COMMON_ENUMS_POKERTYPE_H_

#include <string>
#include <vector>

enum class PokerType{
	BLANK,
	DIAMOND,
	CLUB,
	SPADE,
	HEART
};

const std::vector<std::string> POKERTYPE = {" ", "♦", "♣", "♠", "♥"};

static std::vector<PokerType> getPokerTypes()
{
	return { PokerType::BLANK, PokerType::CLUB, PokerType::DIAMOND,
			 PokerType::HEART, PokerType::SPADE};
}


#endif /* LANDLORDS_COMMON_ENUMS_POKERTYPE_H_ */
