/*
 * SellType.h
 *
 *  Created on: 2021年2月20日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENUMS_SELLTYPE_H_
#define LANDLORDS_COMMON_ENUMS_SELLTYPE_H_

#include <vector>
#include <string>

enum SellType {
	ILLEGAL,               // 非合法
	BOMB,                  // 炸弹
	KING_BOMB,             // 王炸
	SINGLE,                // 单个牌
	DOUBLE,                // 对子
	THREE,                 // 三张牌
	THREE_ZONES_SINGLE,    // 三带单
	THREE_ZONES_DOUBLE,    // 三带一对
	FOUR_ZONES_SINGLE,     // 四带单
	FOUR_ZONES_DOUBLE,     // 四带一对
	SINGLE_STRAIGHT,       // 单顺子
	DOUBLE_STRAIGHT,       // 双顺子
	THREE_STRAIGHT,        // 三顺子
	FOUR_STRAIGHT,         // 四顺子
	THREE_STRAIGHT_WITH_SINGLE,    // 飞机带单牌
	THREE_STRAIGHT_WITH_DOUBLE,    // 飞机带对牌
	FOUR_STRAIGHT_WITH_SINGLE,     // 四顺子带单
	FOUR_STRAIGHT_WITH_DOUBLE,     // 四顺子带对
};

const std::vector<std::string> SHELLTYPE = { "非合法", "炸弹", "王炸", "单个牌", "对子", "三张牌",
			"三带单", "三带一对", "四带单", "四带一对", "单顺子", "双顺子", "三顺子", "四顺子",
			"飞机带单牌", "飞机带对牌", "四顺子带单", "四顺子带对"};

#endif /* LANDLORDS_COMMON_ENUMS_SELLTYPE_H_ */
