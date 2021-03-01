/*
 * answers.h
 *
 *  Created on: 2021年2月10日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_ANSWERS_H_
#define LANDLORDS_SERVER_ANSWERS_H_

#include <map>
#include "protobuf/query.pb.h"
#include "helper/PokerHelper.h"
#include "enums/PokerBasic.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

enum ANSWERNO {
	INITIALIZE
};

std::string initialize(const muduo::Query &query,
					   const std::vector<Poker> &pokers)
{
	return PokerHelper::printPokers(pokers);
}

std::string showPokers(const muduo::Query &query,
		   const std::vector<Poker> &pokers)
{
	return PokerHelper::printPokers(pokers);
}

bool questionToMap(const std::string &question, std::map<PokerLevel, int> &result)
{
	// ShellType::ILLEGAL时返回 false
	for (auto iter = question.cbegin(); iter != question.cend();
			++iter)
	{
		switch(*iter)
		{
		case '3':
			result[PokerLevel::LEVEL_3] += 1;
			break;
		case '4':
			result[PokerLevel::LEVEL_4] += 1;
			break;
		case '5':
			result[PokerLevel::LEVEL_5] += 1;
			break;
		case '6':
			result[PokerLevel::LEVEL_6] += 1;
			break;
		case '7':
			result[PokerLevel::LEVEL_7] += 1;
			break;
		case '8':
			result[PokerLevel::LEVEL_8] += 1;
			break;
		case '9':
			result[PokerLevel::LEVEL_9] += 1;
			break;
		case '1':
			result[PokerLevel::LEVEL_10] += 1;
			++iter;
			break;
		case '0':
			return false;
		case 'j':
		case 'J':
			result[PokerLevel::LEVEL_J] += 1;
			break;
		case 'q':
		case 'Q':
			result[PokerLevel::LEVEL_Q] += 1;
			break;
		case 'k':
		case 'K':
			result[PokerLevel::LEVEL_K] += 1;
			break;
		case 'a':
		case 'A':
			result[PokerLevel::LEVEL_A] += 1;
			break;
		case '2':
			result[PokerLevel::LEVEL_2] += 1;
			break;
		case 'S':
			result[PokerLevel::LEVEL_SMALL_KING] += 1;
			break;
		case 'X':
			result[PokerLevel::LEVEL_BIG_KING] += 1;
			break;
		default:
			return false;
		}
	}
}

bool isValid()
{
	return true;
}

std::map<PokerLevel, int> parseRequest(const std::string &question)
{
	std::map<PokerLevel, int> result;
	if (!questionToMap(question, result))
		return ShellType::ILLEGAL;
	if (isValid())
	{
		return result;
	}
	return NULL;
}

std::string play(const muduo::Query &query,
		    const std::vector<Poker> &pokers)
{
	for (const auto &p: pokers)
	{

	}
}

std::string processRequest(const muduo::Query &query,
		   const std::vector<Poker> &pokers)
{
	switch(query.id())
	{
	case ShellType::SINGLE:

	}
}

std::function<std::string(const muduo::Query &, const std::vector<Poker> &)> init = initialize;
typedef std::function<std::string(const muduo::Query &, const std::vector<Poker> &)> answerCallback_;

std::vector<answerCallback_> answers_{ init, };

#endif /* LANDLORDS_SERVER_ANSWERS_H_ */
