/*
 * MapHelper.h
 *
 *  Created on: 2021年2月16日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_HELPER_MAPHELPER_H_
#define LANDLORDS_COMMON_HELPER_MAPHELPER_H_

#include "xpack/json.h"
#include <unordered_map>


template <class T>
class MapHelper {
private:
	std::unordered_map<std::string, T> data_;

public:
	static MapHelper<T> newInstance()
	{
		return MapHelper();
	}

	static std::unordered_map<std::string, T> parser(std::string json)
	{
		std::unordered_map<std::string, T> res;
		xpack::json::decode(json, res);

	}

private:
	MapHelper()
		: data_(std::unordered_map<std::string, T>())
	{};

public:
	MapHelper();
	virtual ~MapHelper();
};

#endif /* LANDLORDS_COMMON_HELPER_MAPHELPER_H_ */
