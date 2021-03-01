/*
 * Poker.h
 *
 *  Created on: 2021年2月5日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_POKER_H_
#define LANDLORDS_COMMON_POKER_H_

#include <map>
#include <string>

#include "PokerBasic.h"
class Poker {
public:
	Poker() : type_(PokerType::BLANK), level_(PokerLevel::LEVEL_3)  {}
	Poker(PokerBasic::PokerType type, PokerBasic::PokerLevel level)
		: type_(type), level_(level)
	{
	}
	~Poker(){};

	// getter and setter
	int getType() { return int(type_); }
	int getLevel() { return int(level_); }

	std::string getTypeStr() { return PokerBasic::POKERTYPE[int(type_)]; }
	std::string getLevelStr() { return PokerBasic::POKERLEVEL[int(level_)]; }

	void setType(PokerBasic::PokerType type) { this->type_ = type; }
	void setLevel(PokerBasic::PokerLevel level) { this->level_ = level_; }

	// toString
	std::string toString()
	{
		return PokerBasic::POKERLEVEL[int(level_)] + " " + PokerBasic::POKERTYPE[int(type_)];
	}



	// operator ==
	bool operator ==(const Poker & rhs)
	{
		return type_ == rhs.type_ &&
				level_ == rhs.level_;
	}

	// operator <=
	bool operator <=(const Poker & rhs)
	{
		if (level_ < rhs.level_)
			return true;

		if (type_ <= rhs.type_)
			return true;
		return false;
	}

public:
	PokerBasic::PokerType type_;
	PokerBasic::PokerLevel level_;
};

namespace boost {
	namespace serialization {

		template<class Archive>
		void serialize(Archive & ar, Poker & d, const unsigned int version)
		{
			// serialize base class information
			ar & d.level_;
			ar & d.type_;
		}

	} // namespace serialization
} // namespace boost

#endif /* LANDLORDS_COMMON_POKER_H_ */
