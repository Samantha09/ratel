/*
 * RoomType.h
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENUMS_ROOMTYPE_H_
#define LANDLORDS_COMMON_ENUMS_ROOMTYPE_H_

#include <vector>
#include <string>

enum class RoomType {
	PVP,
	PVE
};

static std::vector<std::string> ROOMTYPESTRLIST = {"玩家对战", "人机对战"};


static std::string getRoomTypeStr(RoomType type)
{
	return ROOMTYPESTRLIST[int(type)];
}

#endif /* LANDLORDS_COMMON_ENUMS_ROOMTYPE_H_ */
