/*
 * ServerEventCode.h
 *
 *  Created on: 2021年2月16日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENUMS_SERVEREVENTCODE_H_
#define LANDLORDS_COMMON_ENUMS_SERVEREVENTCODE_H_

#include <vector>
#include <string>

enum class ServerEventCode {
	CODE_CLIENT_EXIT,                             //  ("玩家退出")
	CODE_CLIENT_OFFLINE,                          // ("玩家离线"),
	CODE_CLIENT_NICKNAME_SET,                     // ("设置昵称"),
	CODE_CLIENT_HEAD_BEAT,                        // ("不出"),
	CODE_ROOM_CREATE,                             // ("创建PVP房间"),
	CODE_ROOM_CREATE_PVE,                         // ("创建PVE房间"),
	CODE_GET_ROOMS,                               // ("获取房间列表"),
	CODE_ROOM_JOIN,                               // ("加入房间"),
	CODE_GAME_STARTING,                           // ("游戏开始"),
	CODE_GAME_LANDLORD_ELECT,                     // ("抢地主"),
	CODE_GAME_POKER_PLAY,                         // ("出牌环节"),
	CODE_GAME_POKER_PLAY_REDIRECT,                // ("出牌重定向"),
	CODE_GAME_POKER_PLAY_PASS,                    // ("不出"),
	CODE_GAME_WATCH,                              // ("观战");
};

const std::vector<std::string> SERVEREVENTCODE = { "玩家退出", "玩家离线", "设置昵称", "不出",
		    "创建PVP房间", "创建PVE房间", "获取房间列表", "加入房间", "游戏开始", "抢地主", "出牌环节",
			"出牌重定向", "不出", "观战" };


#endif /* LANDLORDS_COMMON_ENUMS_SERVEREVENTCODE_H_ */
