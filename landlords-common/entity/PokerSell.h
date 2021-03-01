/*
 * PokerSell.h
 *
 *  Created on: 2021年2月20日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENUMS_POKERSELL_H_
#define LANDLORDS_COMMON_ENUMS_POKERSELL_H_

#include "enums/SellType.h"
#include <vector>
#include "entity/Poker.h"
//#include "helper/PokerHelper.h"

class PokerSell {
private:
	int score_;
	SellType sellType_;
	std::vector<Poker> sellPokers_;
	int coreLevel_;

public:
	PokerSell(SellType sellType, std::vector<Poker> sellPokers, int coreLevel);
	// 有点不太理解这个
	int getCoreLevel()
	{
		return coreLevel_;
	}

	void setCoreLevel(int coreLevel)
	{
		coreLevel_ = coreLevel;
	}

	int getScore()
	{
		return score_;
	}

	void setScore(int score)
	{
		score_ = score;
	}

	SellType getSellType()
	{
		return sellType_;
	}

	void setSellType(SellType sellType)
	{
		sellType_ = sellType;
	}

	std::vector<Poker> *getSellPokers()
	{
		return &sellPokers_;
	}

	void setSellPokers(std::vector<Poker> sellPokers)
	{
		sellPokers_ = sellPokers;
	}

//	std::string toString()
//	{
//		return sellType_ + "\t| " + score_ + sellPokers_;
//	}



};


#endif /* LANDLORDS_COMMON_ENUMS_POKERSELL_H_ */
