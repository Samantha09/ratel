/*
 * ClientEventCode.h
 *
 *  Created on: 2021年2月11日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENUMS_CLIENTEVENTCODE_H_
#define LANDLORDS_COMMON_ENUMS_CLIENTEVENTCODE_H_

#include <vector>
#include <string>

enum class ClientEventCode {
	CODE_CLIENT_NICKNAME_SET,                      // ("设置昵称"),
	CODE_CLIENT_EXIT,                              // ("客户端退出"),
	CODE_CLIENT_KICK,                              // ("客户端被踢出"),
	CODE_CLIENT_CONNECT,                           // ("客户端加入成功"),
	CODE_SHOW_OPTIONS,                             // ("全局选项列表"),
	CODE_SHOW_OPTIONS_SETTING,                     // ("设置选项"),
	CODE_SHOW_OPTIONS_PVP,                         // ("玩家对战选项"),
	CODE_SHOW_OPTIONS_PVE,                         // ("人机对战选项"),
	CODE_SHOW_ROOMS,                               // ("展示房间列表"),
	CODE_SHOW_POKERS,                              // ("展示Poker"),
	CODE_ROOM_CREATE_SUCCESS,                      // ("创建房间成功"),
	CODE_ROOM_JOIN_SUCCESS,                        // ("加入房间成功"),
	CODE_ROOM_JOIN_FAIL_BY_FULL,                   // ("房间人数已满"),
	CODE_ROOM_JOIN_FAIL_BY_INEXIST,                // ("加入-房间不存在"),
	CODE_ROOM_PLAY_FAIL_BY_INEXIST1,               // ("出牌-房间不存在"),
	CODE_GAME_STARTING,                             //  ("开始游戏"),
	CODE_GAME_LANDLORD_ELECT,                       // ("抢地主"),
	CODE_GAME_LANDLORD_CONFIRM,                     //  ("地主确认"),
	CODE_GAME_LANDLORD_CYCLE,                       // ("地主一轮确认结束"),
	CODE_GAME_POKER_PLAY,                           // ("出牌回合"),
	CODE_GAME_POKER_PLAY_REDIRECT,                  // ("出牌重定向"),
	CODE_GAME_POKER_PLAY_MISMATCH,                  // ("出牌不匹配"),
	CODE_GAME_POKER_PLAY_LESS,                      // ("出牌太小"),
	CODE_GAME_POKER_PLAY_PASS,                      // ("不出"),
	CODE_GAME_POKER_PLAY_CANT_PASS,                 // ("不允许不出"),
	CODE_GAME_POKER_PLAY_INVALID,                   // ("无效"),
	CODE_GAME_POKER_PLAY_ORDER_ERROR,               // ("顺序错误"),
	CODE_GAME_OVER,                                 // ("游戏结束"),
	CODE_PVE_DIFFICULTY_NOT_SUPPORT,                // ("人机难度不支持"),
	CODE_GAME_WATCH,                                // ("观战"),
	CODE_GAME_WATCH_SUCCESSFUL,                     // ("观战成功");
	CODE_GAME_ID_SET                                // ("设置ID")
};

const std::vector<std::string> clientEventCodeToString = { "设置昵称", "客户端退出", "客户端被踢出",
		"客户端加入成功", "全局选项列表", "设置选项", "玩家对战选项", "人机对战选项", "展示房间列表",
		"展示Poker", "创建房间成功", "加入房间成功", "房间人数已满", "加入-房间不存在", "出牌-房间不存在",
		"开始游戏", "抢地主", "地主确认", "地主一轮确认结束", "出牌回合", "出牌重定向", "出牌不匹配",
		"出牌太小", "不出", "不允许不出", "无效", "顺序错误", "游戏结束", "人机难度不支持",
		"观战", "观战成功"};

#endif /* LANDLORDS_COMMON_ENUMS_CLIENTEVENTCODE_H_ */
