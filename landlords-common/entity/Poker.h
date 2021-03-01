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

#include "enums/PokerType.h"
#include "enums/PokerLevel.h"
#include "boost/serialization/serialization.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"


class Poker {
public:
	Poker() : type_(PokerType::BLANK), level_(PokerLevel::LEVEL_3)  {}
	Poker(const Poker& p)
	{
		type_ = p.type_;
		level_ = p.level_;
	}
	Poker(PokerType type, PokerLevel level)
		: type_(type), level_(level)
	{
	}
	~Poker(){};

	// getter and setter
	int getType() { return int(type_); }
	int getLevel() { return int(level_); }

	std::string getTypeStr() { return POKERTYPE[int(type_)]; }
	std::string getLevelStr() { return POKERLEVEL[int(level_)]; }

	void setType(PokerType type) { this->type_ = type; }
	void setLevel(PokerLevel level) { this->level_ = level_; }

	// toString
	std::string toString()
	{
		return POKERLEVEL[int(level_)] + " " + POKERTYPE[int(type_)];
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & type_;
		ar & level_;
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
	PokerType type_;
	PokerLevel level_;
};

#endif /* LANDLORDS_COMMON_POKER_H_ */
