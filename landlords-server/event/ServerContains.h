/*
 * ServerContains.h
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */

#ifndef LANDLORDS_SERVER_SERVERCONTAINS_H_
#define LANDLORDS_SERVER_SERVERCONTAINS_H_

#include "muduo/base/Singleton.h"
#include <unordered_map>
#include <atomic>
#include "entity/Room.h"
#include "entity/ClientSide.h"

#include "muduo/base/Logging.h"

class ServerContains {

public:
	/*
	 * Server port
	 */
	static int port;

	/*
	 * the list of clientSide
	 */
	static std::unordered_map<int, ClientSide*> CLIENT_SIDE_MAP;
	static std::atomic<int> CLIENT_ATOMIC_ID;
	static std::atomic<int> SERVER_ATOMIC_ID;

	static int getClientId()
	{
		return ++CLIENT_ATOMIC_ID;
	}

	static int getServerId()
	{
		return ++SERVER_ATOMIC_ID;
	}

	static Room *getRoom(int id)
	{
		LOG_DEBUG << "getRoom";
		auto roomIter = ROOM_MAP.find(id);
		if (roomIter != ROOM_MAP.end())
		{
			LOG_DEBUG << "roomIter != ROOM_MAP.end()";
			return roomIter->second;
		}
		LOG_DEBUG << "roomIter == ROOM_MAP.end()";
		return roomIter->second;
	}

	static void addRoom(Room *room)
	{
		ROOM_MAP.insert(std::make_pair(room->getId(), room));
	}

	static bool removeRoom(int id)
	{
		return ROOM_MAP.erase(id);
	}

private:
	/*
	 * the map of server side
	 */
	static std::unordered_map<int, Room*> ROOM_MAP;

public:
	ServerContains(){};
	virtual ~ServerContains(){};
};

//ServerContains& serverContains = muduo::Singleton<ServerContains>::instance();

#endif /* LANDLORDS_SERVER_SERVERCONTAINS_H_ */
