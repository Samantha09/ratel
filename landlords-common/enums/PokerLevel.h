/*
 * PokerLevel.h
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENUMS_POKERLEVEL_H_
#define LANDLORDS_COMMON_ENUMS_POKERLEVEL_H_

#include <string>
#include <vector>

enum class PokerLevel{
	LEVEL_3,
	LEVEL_4,
	LEVEL_5,
	LEVEL_6,
	LEVEL_7,
	LEVEL_8,
	LEVEL_9,
	LEVEL_10,
	LEVEL_J,
	LEVEL_Q,
	LEVEL_K,
	LEVEL_A,
	LEVEL_2,
	LEVEL_SMALL_KING,
	LEVEL_BIG_KING,
	INVALID,
};

const std::vector<std::string> POKERLEVEL = {"3", "4", "5", "6", "7", "8", "9",
		   "10", "J", "Q", "K", "A", "2", "S", "X"};

static std::vector<PokerLevel> getPokerLevels()
{
	return {PokerLevel::LEVEL_3, PokerLevel::LEVEL_4, PokerLevel::LEVEL_5,
			PokerLevel::LEVEL_6, PokerLevel::LEVEL_7, PokerLevel::LEVEL_8,
			PokerLevel::LEVEL_9, PokerLevel::LEVEL_10, PokerLevel::LEVEL_J,
			PokerLevel::LEVEL_Q, PokerLevel::LEVEL_K, PokerLevel::LEVEL_A,
			PokerLevel::LEVEL_2, PokerLevel::LEVEL_SMALL_KING, PokerLevel::LEVEL_BIG_KING};
}


#endif /* LANDLORDS_COMMON_ENUMS_POKERLEVEL_H_ */
