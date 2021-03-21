/*
 * ServerContains.cc
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */


#include "ServerContains.h"

int ServerContains::port = 1024;

std::unordered_map<int, std::shared_ptr<ClientSide> > ServerContains::CLIENT_SIDE_MAP =
					std::unordered_map<int, std::shared_ptr<ClientSide> >();
std::unordered_map<int, std::shared_ptr<Room> > ServerContains::ROOM_MAP = std::unordered_map<int, std::shared_ptr<Room> >();
std::atomic<int> ServerContains::CLIENT_ATOMIC_ID(0);
std::atomic<int> ServerContains::SERVER_ATOMIC_ID(0);

