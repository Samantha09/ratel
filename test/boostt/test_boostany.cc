/*
 * test_boostany.cc
 *
 *  Created on: 2021年3月1日
 *      Author: san
 */

#include "boost/any.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include "helper/SerializeHelper.h"


int main()
{
	MapHelper mh;
	mh.put("id", "12345");
	std::vector<Poker> pokers;
	pokers.push_back(Poker(PokerType::CLUB, PokerLevel::LEVEL_10));
	mh.put("pokers", pokers);
	mh.put("clientId", 1);
	std::string data = SerializeHelper::SerializeToString<MapHelper>(mh);
	MapHelper result = SerializeHelper::parseStringToData<MapHelper>(data);
	int res;
	result.get("clientId", res);
	std::cout << res;
}


