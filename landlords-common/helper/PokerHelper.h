/*
 * PokerHelper.h
 *
 *  Created on: 2021年2月7日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_POKERHELPER_H_
#define LANDLORDS_COMMON_POKERHELPER_H_

#include <vector>
#include <functional>
#include "enums/SellType.h"
#include "entity/PokerSell.h"
#include <unordered_set>
#include <algorithm>

#include "muduo/base/Logging.h"

//class PokerSell;
class Poker;
class PokerHelper {
public:
	typedef std::function<bool (Poker o1, Poker o2)> Comparator;

	PokerHelper();
	virtual ~PokerHelper();

	static void removeAll(std::vector<Poker>& pokers, std::vector<Poker> &currentPokers)
	{
		LOG_WARN << "空函数";
	}

	static std::vector<Poker> getPoker(const std::vector<int> &indexes,
									   const std::vector<Poker> pokers)
	{
		LOG_DEBUG << "getPoker";
		std::vector<Poker> resultPokers;
		for (int index: indexes)
		{
			// LOG_INFO << 这里没有减1
			resultPokers.push_back(pokers.at(index));
		}

		sortPoker(resultPokers);
		return resultPokers;
	}

	static bool checkPokerIndex(const std::vector<int> &indexes,
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
				if (index > pokers.size() || index < 1)
				{
					access = false;
				}
			}
		}
		return access;
	}

	static std::vector<int> getIndexes(const std::vector<PokerLevel> &options, const std::vector<Poker> &pokers)
	{
		// FIXME
		LOG_INFO << "ingore types";
		std::vector<int> indexes(options.size());
		std::vector<Poker> copyList(pokers);
		LOG_INFO << "copyList.size(): " << copyList.size();
		for (int index = 0; index < options.size(); ++index)
		{
			LOG_INFO << "index: " << index;
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
					LOG_INFO << "poker.level_: " << int(poker.level_);
					LOG_INFO << "option: " << int(option);
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
				LOG_INFO << "isTarget: " << isTarget;
				LOG_INFO << "return indexes.size(): " << indexes.size();
				return std::vector<int>();
			}
		}
		std::sort(indexes.begin(), indexes.end());
//		LOG_INFO << "return indexes.size(): " << indexes.size();
		return indexes;
	}

	static void init()
	{
		std::vector<PokerLevel> pokerLevels(getPokerLevels());
		std::vector<PokerType> pokerTypes(getPokerTypes());

		for(PokerLevel level: pokerLevels)
		{
			if (level == PokerLevel::LEVEL_BIG_KING)
			{
				basePokers_.push_back(Poker(PokerType::BLANK, level));
				continue;
			}
			if (level == PokerLevel::LEVEL_SMALL_KING)
			{
				basePokers_.push_back(Poker(PokerType::BLANK, level));
				continue;
			}
			for(PokerType type: pokerTypes)
			{
				if(type == PokerType::BLANK)
				{
					continue;
				}
				basePokers_.push_back(Poker(type, level));
			}
		}
	}

	static std::vector<std::vector<Poker> > distributePoker()
	{

		std::random_shuffle(basePokers_.begin(), basePokers_.end());
		std::vector<std::vector<Poker> > pokersList;
		std::vector<Poker> pokers1;
		pokers1.reserve(17);
		std::copy(basePokers_.begin(),              // 插入到
				  basePokers_.begin() + 17,
				  std::back_inserter(pokers1));
		std::vector<Poker> pokers2;
		pokers2.reserve(17);
		std::copy(basePokers_.begin() + 17,              // 插入到
				  basePokers_.begin() + 34,
				  std::back_inserter(pokers2));
		std::vector<Poker> pokers3;
		pokers3.reserve(17);
		std::copy(basePokers_.begin() + 34,              // 插入到
				  basePokers_.begin() + 51,
				  std::back_inserter(pokers3));
		std::vector<Poker> pokers4;                  // 底牌
		pokers4.reserve(3);
		std::copy(basePokers_.begin() + 51,              // 插入到
				  basePokers_.end(),
				  std::back_inserter(pokers4));
		pokersList.push_back(pokers1);
		pokersList.push_back(pokers2);
		pokersList.push_back(pokers3);
		pokersList.push_back(pokers4);
		for (auto &pokers: pokersList)
		{
			sortPoker(pokers);
		}
		return pokersList;
	}

	static void sortPoker(std::vector<Poker> &pokers);

	static std::string printPokers(std::vector<Poker> pokers);

	// 根据 pokers 生成 pokerShell
	static PokerSell checkPokerType(std::vector<Poker> pokers);


	static std::vector<PokerSell> validSells(PokerSell lastPokerSell, std::vector<Poker> &pokers);

	// parses funcs
	static int parseScore(SellType sellType, int level);
	static std::vector<PokerSell> parsePokerSells(std::vector<Poker> pokers);
	static void parsePokerSellStraight(std::vector<PokerSell> pokerSells, SellType sellType);

private:
	static std::string buildHandStringRounded(std::vector<Poker> pokers);
	static void parseArgs(std::vector<PokerSell> &pokerSells,
			PokerSell &pokerSell,
			int deep,
			SellType sellType,
			SellType targetSellType)
	{
		LOG_INFO << "parseArgs";
		std::unordered_set<int> existLevelSet = std::unordered_set<int>();
		for (auto &p: *(pokerSell.getSellPokers()))
		{
			existLevelSet.insert(p.getLevel());
		}
		// FIXME: 使用了 new
		std::unordered_set<std::vector<Poker>*> temp = std::unordered_set<std::vector<Poker>*>();
		parseArgs(&existLevelSet, pokerSells, temp, pokerSell, deep, sellType, targetSellType);
	}

	static void parseArgs(std::unordered_set<int> *existLevelSet,
			std::vector<PokerSell> &pokerSells,
			std::unordered_set<std::vector<Poker>*> &pokersList,
			PokerSell &pokerSell,
			int deep, SellType sellType, SellType targetSellType)
	{
		if (deep == 0)
		{
			std::vector<Poker> allPokers{};
			std::copy((*pokerSell.getSellPokers()).begin(),              // 插入到
					  (*pokerSell.getSellPokers()).end(),
					  std::back_inserter(allPokers));
			for (auto ps: pokersList)
			{
				LOG_INFO << "PokerHelper::139";
				std::copy(ps->begin(),              // 插入到
						  ps->end(),
						  std::back_inserter(allPokers));
			}
			pokerSells.push_back(PokerSell(targetSellType, allPokers, pokerSell.getCoreLevel()));
			return;
		}

		LOG_INFO << pokerSells.size();
		for (int index = 0; index < pokerSells.size(); ++index)
		{
			LOG_INFO << deep;
			LOG_INFO << "PokerHelper::149 " << index;
			PokerSell &subSell = pokerSells.at(index);
			if (subSell.getSellType() == sellType &&
				existLevelSet->find(subSell.getCoreLevel()) == existLevelSet->end())
			{
				LOG_INFO << "154 " << existLevelSet->size();
				pokersList.insert(subSell.getSellPokers());
				existLevelSet->insert(subSell.getCoreLevel());
				parseArgs(existLevelSet, pokerSells, pokersList, pokerSell,
						  deep - 1, sellType, targetSellType);
				existLevelSet->erase(subSell.getCoreLevel());
				pokersList.erase(subSell.getSellPokers());
			}
		}
	}

private:
	static std::vector<Poker> basePokers_;
	Comparator compare_;
};


#endif /* LANDLORDS_COMMON_POKERHELPER_H_ */
