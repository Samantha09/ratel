/*
 * PokerHelper.cc
 *
 *  Created on: 2021年2月7日
 *      Author: san
 */

#include "PokerHelper.h"
//#include "PokerSell.h"
#include <algorithm>
#include "entity/Poker.h"
//#include "muduo/base/Logging.h"
#include <iostream>

std::vector<Poker> PokerHelper::basePokers_ = std::vector<Poker>();
PokerHelper::PokerHelper() {
	// TODO Auto-generated constructor stub
}

void PokerHelper::removeAll(std::vector<Poker>& pokers, std::vector<Poker> &currentPokers)
{
//		LOG_WARN << "空函数";
	LOG_WARN << "currentPokers.size(): " << currentPokers.size();
	for (auto p: currentPokers)
	{
		auto res = std::find_if(pokers.begin(), pokers.end(), [p](Poker t){return p.level_ == t.level_;});
		if (res != pokers.end())
		{
			pokers.erase(res);
		}
	}
}

std::vector<Poker> PokerHelper::getPoker(const std::vector<int> &indexes,
						                 const std::vector<Poker> pokers)
{
//	LOG_DEBUG << "getPoker";
	std::vector<Poker> resultPokers;
	for (int index: indexes)
	{
		// LOG_INFO << 这里没有减1
		resultPokers.push_back(pokers.at(index));
	}

	sortPoker(resultPokers);
	return resultPokers;
}

bool PokerHelper::checkPokerIndex(const std::vector<int> &indexes,
							      const std::vector<Poker> &pokers)
{
	bool access = true;
	if (indexes.empty())
	{
		access = false;
	}
	else
	{
		for (int index: indexes)
		{
			if (index > pokers.size() || index < 0)
			{
//				LOG_DEBUG << "index: " << index << " pokers.size(): " << pokers.size();
				access = false;
			}
		}
	}
	return access;
}

std::vector<int> PokerHelper::getIndexes(const std::vector<PokerLevel> &options, const std::vector<Poker> &pokers)
{
	// FIXME
	std::vector<int> indexes(options.size());
	std::vector<Poker> copyList(pokers);
	for (int index = 0; index < options.size(); ++index)
	{
//		LOG_INFO << "index: " << index;
		PokerLevel option = options.at(index);
		bool isTarget = false;
		auto iter = copyList.begin();
		while (iter != copyList.end())
		{
//				LOG_INFO << "copyList.size(): " << copyList.size();
//				LOG_INFO << "iter_index: " << std::distance(copyList.begin() ,iter);
			Poker poker = *iter;
			if (poker.level_ == option)
			{
				isTarget = true;
				indexes[index] = std::distance(copyList.begin(), iter);
				iter->level_ = PokerLevel::INVALID;
				break;
			}
			else
			{
				++iter;
			}
		}
		if (!isTarget)
		{
//			LOG_INFO << "isTarget: " << isTarget;
//			LOG_INFO << "return indexes.size(): " << indexes.size();
			return std::vector<int>();
		}
	}
	std::sort(indexes.begin(), indexes.end());
//		LOG_INFO << "return indexes.size(): " << indexes.size();
	return indexes;
}

void PokerHelper::sortPoker(std::vector<Poker> &pokers)
{
	std::sort(pokers.begin(), pokers.end(),
			[](Poker o1, Poker o2){ return o1.getLevel() < o2.getLevel(); });
}

std::string PokerHelper::printPokers(std::vector<Poker> pokers)
{
	return PokerHelper::buildHandStringRounded(pokers);
}


// checkPokerType
PokerSell PokerHelper::checkPokerType(std::vector<Poker> pokers)
{
	if (!pokers.empty())
	{
		sortPoker(pokers);

		std::vector<int> levelTable(20);
		for (Poker poker: pokers)
			levelTable[poker.getLevel()]++;

		int startIndex = -1;
		int endIndex = -1;
		int count = 0;

		int singleCount = 0;
		int doubleCount = 0;
		int threeCount = 0;
		int threeStartIndex = -1;
		int threeEndIndex = -1;
		int fourCount = 0;
		int fourStartIndex = -1;
		int fourEndIndex = -1;
		for (int index = 0; index < levelTable.size(); ++index)
		{
			int value = levelTable[index];
			if (value != 0)
			{
				endIndex = index;
				++count;
				if (startIndex == -1)
					startIndex = index;
				if (value == 1)
					++singleCount;
				else if (value == 2)
					++doubleCount;
				else if (value == 3)
				{
					if (threeStartIndex == -1)
						threeStartIndex = index;
					threeEndIndex = index;
					++threeCount;
				}
				else if (value == 4)
				{
					if (fourStartIndex == -1)
						fourStartIndex = index;
					fourEndIndex = index;
					++fourCount;
				}
			}
		}

		// 炸弹
		if (singleCount == doubleCount &&
			singleCount == threeCount &&
			singleCount == 0 &&
			fourCount == 1)
		{
			return PokerSell(SellType::BOMB, pokers, startIndex);
		}

		// 王炸
		if (singleCount == 2 &&
			startIndex == int(PokerLevel::LEVEL_SMALL_KING) &&
			endIndex == int(PokerLevel::LEVEL_BIG_KING))
		{
			return PokerSell(SellType::KING_BOMB, pokers, int(PokerLevel::LEVEL_SMALL_KING));
		}

		// 单/对/三
		if(startIndex == endIndex) {
			if(levelTable[startIndex] == 1)
			{
				return PokerSell(SellType::SINGLE, pokers, startIndex);
			}
			else if(levelTable[startIndex] == 2)
			{
				return PokerSell(SellType::DOUBLE, pokers, startIndex);
			}
			else if(levelTable[startIndex] == 3)
			{
				return PokerSell(SellType::THREE, pokers, startIndex);
			}
		}

		// 顺子
		if (endIndex - startIndex == count - 1 &&
			endIndex < int(PokerLevel::LEVEL_2))
		{
			if (levelTable[startIndex] == 1 && singleCount > 4 &&
				doubleCount + threeCount + fourCount == 0)
			{  // 单顺子
				return PokerSell(SellType::SINGLE_STRAIGHT, pokers, endIndex);
			}
			else if (levelTable[startIndex] == 2 &&
					 doubleCount > 2 &&
					 singleCount + threeCount + fourCount == 0)
			{  // 双顺子
				return PokerSell(SellType::DOUBLE_STRAIGHT, pokers, endIndex);
			}
			else if (levelTable[startIndex] == 3 &&
					 threeCount > 1 &&
					 doubleCount + singleCount + fourCount == 0)
			{  // 三顺子
				return PokerSell(SellType::THREE_STRAIGHT, pokers, endIndex);
			}
			else if (levelTable[startIndex] == 4 &&
					 fourCount > 1 &&
					 doubleCount + threeCount + singleCount == 0)
			{  // 四顺子
				return PokerSell(SellType::FOUR_STRAIGHT, pokers, endIndex);
			}
		}

		if (threeCount != 0)  // 三顺子带单
		{
			if (singleCount != 0 &&
				singleCount == threeCount &&
				doubleCount == 0 &&
				fourCount == 0)
			{
				if (threeCount == 1)   // 三带一
					return PokerSell(SellType::THREE_ZONES_SINGLE, pokers, threeEndIndex);
				else
				{
					if (threeEndIndex - threeStartIndex + 1 == threeCount &&
						threeEndIndex < int(PokerLevel::LEVEL_2))
					{
						return PokerSell(SellType::THREE_STRAIGHT_WITH_SINGLE, pokers, threeEndIndex);
					}
				}
			}
			else if (doubleCount != 0 &&
					 doubleCount == threeCount &&
					 singleCount == 0 && fourCount == 0)
			{
				if (threeCount == 1)  // 三带一对
					return PokerSell(SellType::THREE_ZONES_DOUBLE, pokers, threeEndIndex);
				else
				{
					if (threeEndIndex - threeStartIndex + 1 == threeCount &&
						threeEndIndex < int(PokerLevel::LEVEL_2))
					{
						return PokerSell(SellType::THREE_STRAIGHT_WITH_DOUBLE, pokers, threeEndIndex);
					}
				}
			}
		}

		// 四带
		if (fourCount != 0)
		{
			if (singleCount != 0 &&
				singleCount == fourCount * 2 &&
				doubleCount == 0 && threeCount == 0)
			{
				if (fourCount == 1)
					return PokerSell(SellType::FOUR_ZONES_SINGLE, pokers, fourEndIndex);
				else
				{
					if(fourEndIndex - fourStartIndex + 1 == fourCount && fourEndIndex < int(PokerLevel::LEVEL_2))
					{
						return PokerSell(SellType::FOUR_STRAIGHT_WITH_SINGLE, pokers, fourEndIndex);
					}
				}
			}
			else if (doubleCount != 0 &&
					 doubleCount == fourCount * 2 && singleCount == 0 &&
					 threeCount == 0)
			{
				if (fourCount == 1)
					return PokerSell(SellType::FOUR_ZONES_DOUBLE, pokers, fourEndIndex);
				else
				{
					if (fourEndIndex - fourStartIndex + 1 == fourCount &&
							fourEndIndex < int(PokerLevel::LEVEL_2))
					{
						return PokerSell(SellType::FOUR_STRAIGHT_WITH_DOUBLE, pokers, fourEndIndex);
					}
				}
			}
		}
	}
	return PokerSell(SellType::ILLEGAL, std::vector<Poker>(), -1);   // 非法请求
}

// validSell
std::vector<PokerSell> PokerHelper::validSells(PokerSell lastPokerSell, std::vector<Poker> &pokers)
{
//	LOG_DEBUG << "validSell 正在解析！";
	std::vector<PokerSell> sells = PokerHelper::parsePokerSells(pokers);
	// FIXME: if(lastPokerSell == null)
	std::cout << sells.size() << std::endl;
	if (lastPokerSell.getSellType() == SellType::VOID_SELL)
		return sells;

	std::vector<PokerSell> validSells;
	for (PokerSell sell: sells)
	{
		if (sell.getSellType() == lastPokerSell.getSellType())
		{
			if (sell.getScore() > lastPokerSell.getScore() &&
				sell.getSellPokers()->size() == lastPokerSell.getSellPokers()->size())
			{
				validSells.push_back(sell);
			}
		}

		if (sell.getSellType() == SellType::KING_BOMB)
		{
			validSells.push_back(sell);
		}
	}

	if (lastPokerSell.getSellType() != SellType::BOMB)
	{
		for (PokerSell sell: sells)
		{
			if (sell.getSellType() == SellType::BOMB)
			{
				validSells.push_back(sell);
			}
		}
	}
	return validSells;
}


// parse funcs
int PokerHelper::parseScore(SellType sellType, int level)
{
	if (sellType == SellType::BOMB)                    // 炸弹
	{
		return level * 4 + 999;
	}
	else if (sellType == SellType::KING_BOMB)          // 王炸
	{
		return INT32_MAX;
	}
	else if(sellType == SellType::SINGLE ||            // 单/双/三个
			sellType == SellType::DOUBLE ||
			sellType == SellType::THREE)
	{
		return level;
	}
	else if(sellType == SellType::SINGLE_STRAIGHT ||
			sellType == SellType::DOUBLE_STRAIGHT ||
			sellType == SellType::THREE_STRAIGHT ||
			sellType == SellType::FOUR_STRAIGHT)
	{
		return level;
	}
	else if(sellType == SellType::THREE_ZONES_SINGLE ||
			sellType == SellType::THREE_STRAIGHT_WITH_SINGLE ||
			sellType == SellType::THREE_ZONES_DOUBLE ||
			sellType == SellType::FOUR_STRAIGHT_WITH_DOUBLE)
	{
		return level;
	}
	else if(sellType == SellType::FOUR_ZONES_SINGLE ||
			sellType == SellType::FOUR_STRAIGHT_WITH_SINGLE ||
			sellType == SellType::FOUR_ZONES_DOUBLE ||
			sellType == SellType::FOUR_STRAIGHT_WITH_DOUBLE)
	{
		return level;
	}
	return -1;                                                    // error
}

void PokerHelper::parsePokerSellStraight(std::vector<PokerSell> pokerSells, SellType sellType)
{
	/*
	 * 处理顺子
	 */
//	LOG_INFO << "parsePokerSellStraight";
	int minLenght = -1;
	int width = -1;
	SellType targetSellType;                    // C++中是否合适
	if (sellType == SellType::SINGLE)                  // 单牌顺子
	{
		minLenght = 5;
		width = 1;
		targetSellType = SellType::SINGLE_STRAIGHT;
	}
	else if (sellType == SellType::DOUBLE)             // 连对
	{
		minLenght = 3;
		width = 2;
		targetSellType = SellType::DOUBLE_STRAIGHT;
	}
	else if (sellType == SellType::THREE)               // 飞机
	{
		minLenght = 2;
		width = 3;
		targetSellType = SellType::THREE_STRAIGHT;
	}
	else if (sellType == SellType::BOMB)                // 四带
	{
		minLenght = 2;
		width = 4;
		targetSellType = SellType::FOUR_STRAIGHT;
	}

	int increase_1 = 0;
	int lastLevel_1 = -1;
	std::vector<Poker> sellPokers_1(4);
	for (int index = 0; index < pokerSells.size(); ++index)
	{
		PokerSell sell = pokerSells.at(index);

		if (sell.getSellType() == sellType)
		{
			int level = sell.getCoreLevel();
			if (lastLevel_1 == -1)
			{
				++increase_1;
				std::copy(sell.getSellPokers()->begin(),              // 插入到
						  sell.getSellPokers()->end(),
					      std::back_inserter(sellPokers_1));
			}
			else
			{
				if (level - 1 == lastLevel_1 && level != int(PokerLevel::LEVEL_2))
				{
					++increase_1;
					std::copy(sell.getSellPokers()->begin(),              // 插入到
							  sell.getSellPokers()->end(),
							  std::back_inserter(sellPokers_1));
				}
				else
				{
					if(increase_1 >= minLenght)
					{
						for(int s = 0; s <= increase_1 - minLenght; s ++)
						{
							int len = minLenght + s;
							for(int subIndex = 0; subIndex <= increase_1 - len; subIndex ++)
							{
								std::vector<Poker> pokers(sellPokers_1.begin() + subIndex * width,
										sellPokers_1.begin() + (subIndex + len) * width);
								// FIXME:
								pokerSells.push_back(PokerSell(targetSellType,
													 pokers,
													 pokers.at(pokers.size() - 1).getLevel()));
							}
						}
					}
					increase_1 = 1;
					sellPokers_1.clear();
					std::copy(sell.getSellPokers()->begin(),              // 插入到
							  sell.getSellPokers()->end(),
							  std::back_inserter(sellPokers_1));
				}
			}
			lastLevel_1 = level;
		}
	}

	// FIXME
	if (!sellPokers_1.empty())
	{
		if (increase_1 >= minLenght)
		{
			for (int s = 0; s <= increase_1 - minLenght; ++s)
			{
				int len = minLenght + s;
				for (int subIndex = 0; subIndex <= increase_1 - len; ++subIndex)
				{
					std::vector<Poker> pokers(sellPokers_1.begin() + subIndex * width,
							sellPokers_1.begin() + (subIndex + len) * width);
					// FIXME:
					pokerSells.push_back(PokerSell(targetSellType,
										 pokers,
										 pokers.at(pokers.size() - 1).getLevel()));
				}
			}
		}
		increase_1 = 0;
		sellPokers_1.clear();
	}
}

std::vector<PokerSell> PokerHelper::parsePokerSells(std::vector<Poker> pokers)
{
	std::cout << "parsePokerSells: " << std::endl;
	std::vector<PokerSell> pokerSells { };
	int size = pokers.size();

	// all single or double
	{
		int count = 0;
		int lastLevel = -1;
		std::vector<Poker> sellPokers;
		for (int index = 0; index < pokers.size(); index++)
		{
			Poker poker = pokers.at(index);
			int level = poker.getLevel();
			if (lastLevel == -1)
			{
				++count;
			}
			else
			{
				if (level == lastLevel)
				{
					++count;
				}
				else
				{
					count = 1;
					sellPokers.clear();
				}
			}

			sellPokers.push_back(poker);
			if (count == 1)
				pokerSells.push_back(PokerSell(SellType::SINGLE, sellPokers, poker.getLevel()));
			else if (count == 2)
				pokerSells.push_back(PokerSell(SellType::DOUBLE, sellPokers, poker.getLevel()));
			else if (count == 3)
				pokerSells.push_back(PokerSell(SellType::THREE, sellPokers, poker.getLevel()));
			else if (count == 4)
				pokerSells.push_back(PokerSell(SellType::BOMB, sellPokers, poker.getLevel()));

			lastLevel = level;
		}
	}

	// shunzi
	{
		parsePokerSellStraight(pokerSells, SellType::SINGLE);
		parsePokerSellStraight(pokerSells, SellType::DOUBLE);
		parsePokerSellStraight(pokerSells, SellType::THREE);
		parsePokerSellStraight(pokerSells, SellType::BOMB);
	}

	// shunzi with args
	{
		for (int index = 0; index < pokerSells.size(); ++index)
		{
			PokerSell sell = pokerSells.at(index);
			if (sell.getSellType() == SellType::THREE)  // 三带一和三带一对
			{
				parseArgs(pokerSells, sell, 1, SellType::SINGLE, SellType::THREE_ZONES_SINGLE);
				parseArgs(pokerSells, sell, 1, SellType::DOUBLE, SellType::THREE_ZONES_DOUBLE);
			}
			else if (sell.getSellType() == SellType::BOMB)   // 四带一，四带一对
			{
				parseArgs(pokerSells, sell, 2, SellType::SINGLE, SellType::FOUR_ZONES_SINGLE);
				parseArgs(pokerSells, sell, 2, SellType::DOUBLE, SellType::FOUR_ZONES_DOUBLE);
			}
			else if (sell.getSellType() == SellType::THREE_STRAIGHT)   // 飞机/顺子带牌
			{
				int count = sell.getSellPokers()->size() / 3;
				parseArgs(pokerSells, sell, count, SellType::SINGLE, SellType::THREE_STRAIGHT_WITH_SINGLE);
				parseArgs(pokerSells, sell, count, SellType::DOUBLE, SellType::THREE_STRAIGHT_WITH_DOUBLE);
			}
			else if (sell.getSellType() == SellType::FOUR_STRAIGHT)    // 四顺子带牌
			{
				int count = (sell.getSellPokers()->size() / 4) * 2;
				parseArgs(pokerSells, sell, count, SellType::SINGLE, SellType::FOUR_STRAIGHT_WITH_SINGLE);
				parseArgs(pokerSells, sell, count, SellType::DOUBLE, SellType::FOUR_STRAIGHT_WITH_DOUBLE);
			}
		}
	}

	// king boom
	{
		if (size > 1)
		{
			if (pokers.at(size - 1).getLevel() == int(PokerLevel::LEVEL_BIG_KING)
			    && pokers.at(size - 2).getLevel() == int(PokerLevel::LEVEL_SMALL_KING))
			{
				std::vector<Poker> temp;
				temp.push_back(pokers.at(size - 2));
				temp.push_back(pokers.at(size - 1));
				pokerSells.push_back(PokerSell(SellType::KING_BOMB,
									 	 	   temp, int(PokerLevel::LEVEL_BIG_KING)));
			}
		}
	}

	return pokerSells;
}

std::string PokerHelper::buildHandStringRounded(std::vector<Poker> pokers)
{
	std::string builder;
	if (!pokers.empty())
	{
		for (int index = 0; index < pokers.size(); ++index)
		{
			if (index == 0)
				builder.append("┌──╮");
			else
				builder.append("──╮");
		}

		builder.append("\n");

		for (int index = 0; index < pokers.size(); ++index)
		{
			if (index == 0)
				builder.append("|");
			std::string name = pokers[index].getLevelStr();
			builder.append(name + (name.length() == 1 ? " ": "") + "|");
		}

		builder.append("\n");

		for (int index = 0; index < pokers.size(); ++index)
		{
			if (index == 0)
				builder.append("|");
			builder.append(pokers[index].getTypeStr() + " |");
		}

		builder.append("\n");

		for (int index = 0; index < pokers.size(); ++index)
		{
			if (index == 0)
				builder.append("└──╯");
			else
				builder.append("──╯");
		}
	}
	return builder;
}

PokerHelper::~PokerHelper() {
	// TODO Auto-generated destructor stub
}

