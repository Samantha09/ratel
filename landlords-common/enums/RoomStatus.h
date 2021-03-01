/*
 * RoomStatus.h
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENUMS_ROOMSTATUS_H_
#define LANDLORDS_COMMON_ENUMS_ROOMSTATUS_H_

#include <vector>
#include <string>

enum class RoomStatus {
	BLANK,
	WAIT,
	STARTING,
};

const static std::vector<std::string> ROOMSTATUSSTR{"空闲", "等待", "开始"};

static std::string getRoomStatusStr(RoomStatus status)
{
	return ROOMSTATUSSTR[int(status)];
}



#endif /* LANDLORDS_COMMON_ENUMS_ROOMSTATUS_H_ */
