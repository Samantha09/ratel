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

	enum class PokerType{
		BLANK,
		DIAMOND,
		CLUB,
		SPADE,
		HEART
	};

	enum class PokerLevel{
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
	TO_CHOOSE_,
	NO_READY_,
	READY_,
	WAIT_,
	CALL_LANDLORD_,
	PLAYING_
	};

	enum ClientRole{
		PLAYER,
		ROBOT
	};




	enum RoomStatus {
		BLANK_,    // 空闲
		WAITING,     // 等待
		STARTING_  // 开始
	};

	enum RoomType {
		PVP,    // 玩家对战
		PVE,   // 人机对战
	};

	static std::vector<PokerLevel> getPokerLevels()
	{
		return {PokerLevel::LEVEL_3, PokerLevel::LEVEL_4, PokerLevel::LEVEL_5,
				PokerLevel::LEVEL_6, PokerLevel::LEVEL_7, PokerLevel::LEVEL_8,
				PokerLevel::LEVEL_9, PokerLevel::LEVEL_10, PokerLevel::LEVEL_J,
				PokerLevel::LEVEL_Q, PokerLevel::LEVEL_K, PokerLevel::LEVEL_A,
				PokerLevel::LEVEL_2, PokerLevel::LEVEL_SMALL_KING, PokerLevel::LEVEL_BIG_KING};
	}

	static std::vector<PokerType> getPokerTypes()
	{
		return { PokerType::BLANK, PokerType::CLUB, PokerType::DIAMOND,
				 PokerType::HEART, PokerType::SPADE};
	}


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
