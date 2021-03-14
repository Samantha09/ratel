/*
 * Room.h
 *
 *  Created on: 2021年2月10日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENTITY_ROOM_H_
#define LANDLORDS_COMMON_ENTITY_ROOM_H_

#include "enums/PokerBasic.h"
#include "ClientSide.h"
#include "enums/RoomStatus.h"
#include "enums/RoomType.h"
#include "PokerSell.h"

#include <string>
#include <map>

class Room {
public:
	typedef std::map<int, ClientSide*> ClientSideMap;
	typedef std::vector<ClientSide*> ClientSideVec;

	long getCreateTime()
	{
		muduo::MutexLockGuard lock(mutex_);
		return createTime_;
	}

	void setCreateTime(long createTime)
	{
		muduo::MutexLockGuard lock(mutex_);
		createTime_ = createTime;
	}

	int getDifficultyCoefficient()
	{
		muduo::MutexLockGuard lock(mutex_);
		return difficultyCoefficient_;
	}

	void setDifficultyCoefficient(int difficultyCoefficient)
	{
		muduo::MutexLockGuard lock(mutex_);
		difficultyCoefficient_ = difficultyCoefficient;
	}


	RoomType getType()
	{
		muduo::MutexLockGuard lock(mutex_);
		return type_;
	}

	void setType(RoomType type)
	{
		muduo::MutexLockGuard lock(mutex_);
		type_ = type;
	}

	PokerSell &getLastPokerShell()
	{
		muduo::MutexLockGuard lock(mutex_);
		return lastPokerSell_;
	}

	void setLastPokerShell(const PokerSell &lastPokerSell)
	{
		muduo::MutexLockGuard lock(mutex_);
		lastPokerSell_ = lastPokerSell;
	}

	int getCurrentSellClient()
	{
		muduo::MutexLockGuard lock(mutex_);
		return currentSellClient_;
	}

	void setCurrentSellClient(int currentSellClient)
	{
		muduo::MutexLockGuard lock(mutex_);
		currentSellClient_ = currentSellClient;
	}

	int getId()
	{
		muduo::MutexLockGuard lock(mutex_);
		return id_;
	}

	void setId(int id)
	{
		muduo::MutexLockGuard lock(mutex_);
		id_ = id;
	}

	RoomStatus getStatus()
	{
		muduo::MutexLockGuard lock(mutex_);
		return status_;
	}

	void setStatus(RoomStatus status)
	{
		muduo::MutexLockGuard lock(mutex_);
		status_ = status;
	}

	ClientSideVec &getClientSideList()
	{
		muduo::MutexLockGuard lock(mutex_);
		return clientSideVec_;
	}

	void setClientSideList(ClientSideVec clientSideList)
	{
		muduo::MutexLockGuard lock(mutex_);
		clientSideVec_ = clientSideList;
	}

	ClientSideMap &getClientSideMap()
	{
		muduo::MutexLockGuard lock(mutex_);
		return clientSideMap_;
	}

	void setClientSideMap(const ClientSideMap &clientSideMap)
	{
		muduo::MutexLockGuard lock(mutex_);
		clientSideMap_ = clientSideMap;
	}

	std::vector<Poker> *getLoadlordPokers()
	{
		muduo::MutexLockGuard lock(mutex_);
		return &landlordPokers_;
	}

	void setLandlordPokers(const std::vector<Poker> &landlordPokers)
	{
		muduo::MutexLockGuard lock(mutex_);
		landlordPokers_ = landlordPokers;
	}

	std::string getRoomOwner()
	{
		muduo::MutexLockGuard lock(mutex_);
		return roomOwner_;
	}

	void setRoomOwner(const std::string roomOwner)
	{
		muduo::MutexLockGuard lock(mutex_);
		roomOwner_ = roomOwner;
	}

	int getFirstClient()
	{
		muduo::MutexLockGuard lock(mutex_);
		return firstSellClient_;
	}

	void setFirstSellClient(int firstSellClient)
	{
		muduo::MutexLockGuard lock(mutex_);
		firstSellClient_ = firstSellClient;
	}

	std::vector<ClientSide> getWatchList()
	{
		muduo::MutexLockGuard lock(mutex_);
		return watcherList_;
	}

	void setLandlordId(int landlordId)
	{
		muduo::MutexLockGuard lock(mutex_);
		landlordId_ = landlordId;
	}

	int getLandlordId()
	{
		muduo::MutexLockGuard lock(mutex_);
		return landlordId_;
	}

	int getLastSellClient()
	{
		muduo::MutexLockGuard lock(mutex_);
		return lastSellClient_;
	}

	void setLastSellClient(int lastSellClient)
	{
		muduo::MutexLockGuard lock(mutex_);
		lastSellClient_ = lastSellClient;
	}

	std::vector<Poker> getLandlordPokers()
	{
		muduo::MutexLockGuard lock(mutex_);
		return landlordPokers_;
	}

	void init()
	{
		roomOwner_ = "";
		status_ = RoomStatus::BLANK;
		type_ = RoomType::PVE;
		clientSideMap_ = ClientSideMap();
		clientSideVec_ = ClientSideVec();
		landlordPokers_ = std::vector<Poker>();
		lastPokerSell_ = PokerSell(SellType(0), std::vector<Poker>(), 0);
		difficultyCoefficient_ = 0;
		lastPokerSell_ = PokerSell(SellType::BOMB, std::vector<Poker>(), 0);
	}

private:
	muduo::MutexLock mutex_;
	int id_;
	std::string roomOwner_;
	RoomStatus status_;
	RoomType type_;
	ClientSideMap clientSideMap_;
	ClientSideVec clientSideVec_;

	int landlordId_ = -1;
	std::vector<Poker> landlordPokers_;

	PokerSell lastPokerSell_;

	int lastSellClient_ = -1;
	int currentSellClient_ = -1;
	int difficultyCoefficient_;

	long lastFlushTime_ = 0;

	long createTime_ = 0;

	int firstSellClient_ = 0;

//	int landlordId = -1;

	std::vector<ClientSide> watcherList_ = std::vector<ClientSide>();


public:
	Room(): Room(0){};
	Room(int id)
	:id_ (id), lastPokerSell_(PokerSell(SellType::BOMB, std::vector<Poker>(), 0))
	{
		// TODO Auto-generated constructor stub
		init();
	}
	virtual ~Room(){};
};

#endif /* LANDLORDS_COMMON_ENTITY_ROOM_H_ */
