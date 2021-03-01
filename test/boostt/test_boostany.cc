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

struct temp {

	temp()
		: id(1), data("")
	{

	}

	int id;
	std::string data;
};

int main()
{
	std::vector<boost::any> objMap;
	temp t;
	std::unordered_map<int, temp> it {std::make_pair(1, t)};
	objMap.push_back(it);
	std::string result =  SerializeHelper::SerializeToString<std::vector<boost::any> >(objMap);
	std::vector<boost::any> data = SerializeHelper::parseStringToData<std::vector<boost::any> >(result);
//	std::cout << data.at(0);
	auto r = data.at(0);
	std::cout << r.type();
}


