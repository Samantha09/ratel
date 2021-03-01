/*
 * PokerBasic.h
 *
 *  Created on: 2021年2月5日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_POKERBASIC_H_
#define LANDLORDS_COMMON_POKERBASIC_H_

#include <vector>
#include <string>

struct PokerBasic {

	enum PokerType{
		BLANK,
		DIAMOND,
		CLUB,
		SPADE,
		HEART
	};

	enum PokerLevel{
		LEVEL_3,
		LEVEL_4,
		LEVEL_5,
		LEVEL_6,
		LEVEL_7,
		LEVEL_8,
		LEVEL_9,
		LEVEL_10,
		LEVEL_J,
		LEVEL_Q,
		LEVEL_K,
		LEVEL_A,
		LEVEL_2,
		LEVEL_SMALL_KING,
		LEVEL_BIG_KING,
	};

	enum ClientType {
		LANDLORD,     // 地主
		PEASANT       // 农民
	};

	enum ClientStatus{
	TO_CHOOSE,
	NO_READY,
	READY,
	WAIT,
	CALL_LANDLORD,
	PLAYING
	};

	enum ClientRole{
		PLAYER,
		ROBOT
	};




	enum RoomStatus {
		BLANK,    // 空闲
		WAIT,     // 等待
		STARTING  // 开始
	};

	enum RoomType {
		PVP,    // 玩家对战
		PVE,   // 人机对战
	};



	static const std::vector<std::string> POKERTYPE;
	static const std::vector<std::string> POKERLEVEL;
	static const std::vector<std::string> SHELLTYPE;
};

typedef typename PokerBasic::PokerLevel PokerLevel;
typedef typename PokerBasic::PokerType PokerType;
typedef typename PokerBasic::ClientType ClientType;
typedef typename PokerBasic::ClientStatus ClientStatus;
typedef typename PokerBasic::ClientRole ClientRole;
//typedef typename PokerBasic::ServerEventCode ServerEventCode;
typedef typename PokerBasic::RoomStatus RoomStatus;
typedef typename PokerBasic::RoomType RoomType;


#endif /* LANDLORDS_COMMON_POKERBASIC_H_ */
