/*
 * EventFuncs.cc
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#include "EventFuncs.h"
#include "robot/RobotDecisionMakers.h"
#include "ServerContains.h"
#include "web/JsonMapHelper.h"
#include "enums/ClientEventCode.h"
#include "helper/PokerHelper.h"
#include "muduo/base/Logging.h"
#include "helper/SerializeHelper.h"
#include "../robot/RobotEventListener.h"
#include <thread>

void robot_elect_landlord(WsCodec *codec,
							const muduo::net::TcpConnectionPtr &conn,
							ClientEventCode code,
							ClientSide *robot,
							const MapHelper &mapHelper);

void pushDataToClient(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
			ClientEventCode code, const MapHelper &mapHelper)
{
	//	  LOG_DEBUG << "pushDataToClient " << int(code);
		  LOG_DEBUG << "客户端ID: " << mapHelper.get("clientId", 0) << " " << clientEventCodeToString[int(code)];
		  codec->sendEvent(conn, code, mapHelper);
}

void ServerEventListener_CODE_ROOM_CREATE(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
										  const MapHelper &data)
{
	int clientId = data.get("clientId", 0);
	std::shared_ptr<ClientSide> clientSide = ServerContains::getClient(clientId);
	std::shared_ptr<Room> room(std::make_shared<Room>(ServerContains::getServerId()));
	room->setStatus(RoomStatus::BLANK);
	room->setType(RoomType::PVP);
	room->setRoomOwner(clientSide->getNickname());
	room->getClientSideMap().insert(std::make_pair<int, std::weak_ptr<ClientSide> >(clientSide->getId(), clientSide));
	room->getClientSideList().push_back(clientSide);
	room->setCurrentSellClient(clientId);
	room->setCreateTime(muduo::Timestamp::now().microSecondsSinceEpoch());
	room->setLastFlushTime(muduo::Timestamp::now().microSecondsSinceEpoch());

	clientSide->setRoomId(room->getId());
	ServerContains::addRoom(room);

	LOG_DEBUG << "当前房间数： " << ServerContains::getRoomMap().size();

	pushDataToClient(codec, conn,
			         ClientEventCode::CODE_ROOM_CREATE_SUCCESS,
					 MapHelper().put("roomId", room->getId()));
}

void ServerEventListener_CODE_GET_ROOMS(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
										const MapHelper &data)
{
	MapHelper result;
	std::vector<RoomInfo> roomInfos;
	for (auto entry: ServerContains::getRoomMap())
	{
		std::shared_ptr<Room> room = entry.second;
		RoomInfo roomInfo(room->getId(),
						  room->getRoomOwner(),
						  room->getClientSideList().size(),
						  int(room->getType()));
		roomInfos.push_back(roomInfo);
	}

	result.put("roomInfos", roomInfos);

	pushDataToClient(codec, conn,
			 	 	 ClientEventCode::CODE_SHOW_ROOMS,
					 result);
}


void ServerEventListener_CODE_ROOM_JOIN(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
										const MapHelper &data)
{
	int clientId = data.get("clientId", 0);
	int roomId = data.get("option", 0);
	std::shared_ptr<ClientSide> clientSide = ServerContains::getClient(clientId);
	std::shared_ptr<Room> room = ServerContains::getRoom(roomId);

	if (room == nullptr)
	{
		pushDataToClient(codec, conn,
				 	 	 ClientEventCode::CODE_ROOM_JOIN_FAIL_BY_INEXIST,
						 MapHelper().put("roomId", roomId));
	}
	else
	{
		if (room->getClientSideList().size() == 3)
		{
			pushDataToClient(codec, conn,
							 ClientEventCode::CODE_ROOM_JOIN_FAIL_BY_FULL,
							 MapHelper().put("roomId", room->getId())
							 	 	 	.put("clientId", clientId)
							 	 	 	.put("roomOwner", room->getRoomOwner()));
		}
		else
		{
			clientSide->setRoomId(room->getId());

			auto &roomClientMap = room->getClientSideMap();
			std::vector<std::weak_ptr<ClientSide> > &roomClientList = room->getClientSideList();

			if (roomClientList.size() > 0)
			{
				roomClientList.at(roomClientList.size() - 1).lock()->setNext(clientSide);
				clientSide->setPre(roomClientList.at(roomClientList.size() - 1).lock());
			}

			roomClientMap.insert(std::make_pair(clientSide->getId(), clientSide));
			roomClientList.push_back(clientSide);

			if (roomClientMap.size() == 3)
			{
				clientSide->setNext(roomClientList.at(0).lock());
				roomClientList.at(0).lock()->setPre(clientSide);

				ServerEventListener_CODE_GAME_STARTING(codec, conn, MapHelper().put("roomId", room->getId())
						                                                       .put("clientId", clientId));
			}
			else
			{
				room->setStatus(RoomStatus::WAIT);

				MapHelper result;
				result.put("joinClientId", clientSide->getId())
					  .put("clientNickname", clientSide->getNickname())
					  .put("roomId", room->getId())
					  .put("roomOwner", room->getRoomOwner())
					  .put("roomClientCount", room->getClientSideList().size());
				for (std::weak_ptr<ClientSide> client: roomClientList)
				{
					if (client.expired())
					{
						LOG_WARN << "Client expired while broadcasting roomJoinSuccess, skipping";
						continue;
					}
					LOG_DEBUG << "client 的 id: " << client.lock()->getId();
					pushDataToClient(codec,
									 client.lock()->getConn(),
									 ClientEventCode::CODE_ROOM_JOIN_SUCCESS,
									 result);
				}
			}
		}
	}



}


void ServerEventListener_CODE_CLIENT_EXIT(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
										  const MapHelper &data)
{
	LOG_DEBUG << "ServerEventListener_CODididE_CLIENT_EXIT";
	conn->shutdown();
}

void ServerEventListener_CODE_CLIENT_NICKNAME_SET(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												  const MapHelper &data)
{
	LOG_INFO << "ServerEventListener_CODE_CLIENT_NICKNAME_SET \n";
	int clientId = data.get("clientId", 0);
//	assert(clientId != 0);
	std::string nickname = data.get("nickName", "");
	assert(!nickname.empty());

	if (clientId != 0 && nickname != "")
	{
		auto clientSide = ServerContains::getClient(clientId);
		clientSide->setNickname(nickname);

		// External robot registration: nickname prefix "robot_"
		if (nickname.rfind("robot_", 0) == 0)
		{
			LOG_INFO << "Registering external robot: " << nickname << " id=" << clientId;
			ServerContains::addRobot(clientSide);
		}

		pushDataToClient(codec, conn,
					     ClientEventCode::CODE_SHOW_OPTIONS,
						 MapHelper());
	}
	else
	{
		MapHelper result;
		// FIXME: 0怎么办
		result.put("invalidLength", nickname.length());
		pushDataToClient(codec, conn,
						 ClientEventCode::CODE_CLIENT_NICKNAME_SET,
						 result);
	}
}

void ServerEventListener_CODE_GAME_POKER_PLAY_PASS(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												   const MapHelper &data)
{
	LOG_DEBUG << "ServerEventListener_CODE_GAME_POKER_PLAY_PASS";
	int clientId = data.get("clientId", 0);
	assert(clientId != 0);
//	ClientSide &clientSide = *(ServerContains::CLIENT_SIDE_MAP.at(clientId));
	std::shared_ptr<ClientSide> clientSide = ServerContains::getClient(clientId);
	std::string type = clientSide->type_ ? "农民" : "地主";
	LOG_DEBUG << "clientId: " << clientId << "type: " << type;
	LOG_DEBUG << "clientSide.getRoomId(): " << clientSide->getRoomId();
	std::shared_ptr<Room> room = ServerContains::getRoom(clientSide->getRoomId());
	assert(ServerContains::getRoom(clientSide->getRoomId()) != NULL);
	assert(ServerContains::CLIENT_SIDE_MAP.at(clientId) != NULL);
	LOG_DEBUG << "room.id: " << room->getId();
	if (room->getCurrentSellClient() == clientSide->getId())
	{
		if (clientSide->getId() != room->getLastSellClient())
		{
			std::shared_ptr<ClientSide> turnClient = clientSide->getNext();
			room->setCurrentSellClient(turnClient->getId());

			for (std::weak_ptr<ClientSide> c: room->getClientSideList())
			{
				if (c.expired())
				{
					LOG_WARN << "Client expired while broadcasting playPass, skipping";
					continue;
				}
				std::shared_ptr<ClientSide> client = c.lock();
				MapHelper result;
				result.put("clientId", clientSide->getId())
					  .put("clientNickname", clientSide->getNickname())
					  .put("nextClientId", turnClient->getId())
					  .put("nextClientNickname", turnClient->getNickname());


				if (client->getRole() == ClientRole::PLAYER)
				{
					pushDataToClient(codec,
								     client->getConn(),
									 ClientEventCode::CODE_GAME_POKER_PLAY_PASS,
									 result);
				}
			}

			// 过牌后轮到人类需要下发 playRedirect（含 clientInfos），否则 web 看不到对手信息
			if (turnClient->getRole() == ClientRole::PLAYER)
			{
				MapHelper redirectData;
				PokerSell lastShell = room->getLastPokerShell();
				std::vector<Poker> lastPokers;
				if (lastShell.getSellPokers() != nullptr)
				{
					lastPokers = *lastShell.getSellPokers();
				}
				redirectData.put("clientId", turnClient->getId())
							.put("lastSellPokers", lastPokers)
							.put("lastSellClientId", room->getLastSellClient());
				ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT(codec,
																			  turnClient->getConn(),
																			  redirectData);
			}
			else
			{
				LOG_DEBUG << "pass 里的逻辑";
				std::thread robot(std::bind(robot_elect_landlord, codec, conn,
												    ClientEventCode::CODE_GAME_POKER_PLAY,
													turnClient.get(), data));
				robot.join();
			}
			// notifyWatcherPlayPass(room, clientSide);
			LOG_WARN << "notifyWatcherPlayPass(room, clientSide);";
		}
		else
		{
			pushDataToClient(codec, conn,
							 ClientEventCode::CODE_GAME_POKER_PLAY_CANT_PASS,
							 MapHelper());
		}
	}
	else
	{
		pushDataToClient(codec, conn,
						 ClientEventCode::CODE_GAME_POKER_PLAY_ORDER_ERROR,
						 MapHelper());
	}
}

void ServerEventListener_CODE_GAME_POKER_PLAY(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  const MapHelper &data)
{
	int clientId = data.get("clientId", 0);
	assert(clientId != 0);
	std::shared_ptr<ClientSide> clientSide = ServerContains::getClient(clientId);

	if (clientId < 0)
		LOG_DEBUG << "我是服务器，现在收到了机器人的出牌申请！";
	else
		LOG_DEBUG << "我是服务器，现在收到了玩家的出牌申请！";
	std::shared_ptr<Room> room = ServerContains::getRoom(clientSide->getRoomId());

	assert(room != nullptr);
	assert(ServerContains::CLIENT_SIDE_MAP.at(clientId) != NULL);

	if (room != nullptr)
	{
		if (room->getCurrentSellClient() == clientSide->getId())
		{
			LOG_WARN << "\n currentSellClient: " << room->getCurrentSellClient()
					 << "clientSide.id: " << clientSide->getId();
			std::vector<PokerLevel> options = data.get("options", std::vector<PokerLevel>());

			std::vector<int> indexes = PokerHelper::getIndexes(options, clientSide->getPokers());

//			assert(indexes.size() != 0);

			if (PokerHelper::checkPokerIndex(indexes, clientSide->getPokers()))
			{
				LOG_DEBUG << "PokerHelper::checkPokerIndex(indexes, clientSide->getPokers()";
				bool sellFlag = true;

				std::vector<Poker> currentPokers = PokerHelper::getPoker(indexes, clientSide->getPokers());
				PokerSell currentPokerShell = PokerHelper::checkPokerType(currentPokers);

				if (currentPokerShell.getSellType() != SellType::ILLEGAL)
				{
					if (room->getLastSellClient() != clientSide->getId())
					{
						// FIXME: // room.getLastPokerShell() != null
						PokerSell lastPokerShell = room->getLastPokerShell();

						if (lastPokerShell.getSellType() != currentPokerShell.getSellType() &&
							lastPokerShell.getSellPokers()->size() != currentPokerShell.getSellPokers()->size() &&
							currentPokerShell.getSellType() != SellType::BOMB &&
							currentPokerShell.getSellType() != SellType::KING_BOMB)
						{
							MapHelper result;
							result.put("playType", int(currentPokerShell.getSellType()))
								  .put("playCount", currentPokerShell.getSellPokers()->size())
								  .put("preType", int(lastPokerShell.getSellType()))
								  .put("preCount", lastPokerShell.getSellPokers()->size());
							sellFlag = false;
							pushDataToClient(codec, conn,
											 ClientEventCode::CODE_GAME_POKER_PLAY_MISMATCH,
											 result);
						}    // 106
						else if (lastPokerShell.getScore() >= currentPokerShell.getScore())
						{
							sellFlag = false;
							MapHelper result;
							result.put("playScore", currentPokerShell.getScore())
								  .put("preScore", lastPokerShell.getScore());
							pushDataToClient(codec, conn,
											 ClientEventCode::CODE_GAME_POKER_PLAY_LESS,
											 result);
						}
					}
				}    // 93
				else
				{
					sellFlag = false;
					pushDataToClient(codec, conn,
									ClientEventCode::CODE_GAME_POKER_PLAY_INVALID,
									MapHelper());
				}

				if (sellFlag)
				{
					if (clientId < 0)
					{
						LOG_DEBUG << "机器人出的牌合法";
					}
					else
					{
						LOG_DEBUG << "玩家出的牌合法";
					}
					std::shared_ptr<ClientSide> next = clientSide->getNext();
					room->setLastSellClient(clientSide->getId());
					room->setCurrentSellClient(next->getId());
					room->setLastPokerShell(currentPokerShell);

					LOG_INFO << "clientSide->getPokers().size(): " << clientSide->getPokers().size();
					PokerHelper::removeAll(clientSide->getPokers(), currentPokers);
					LOG_INFO << "clientSide->getPokers().size(): " << clientSide->getPokers().size();

					MapHelper mapHelper;
					mapHelper.put("clientId", clientSide->getId())
							 .put("nextClientId", next->getId())
							 .put("clientNickname",clientSide->getNickname())
							 .put("clientType", int(clientSide->getType()))
							 .put("pokers", currentPokers)
							 .put("lastSellClientId", clientSide->getId())
							 .put("lastSellPokers", currentPokers);

					if (!clientSide->getPokers().empty())
					{
						mapHelper.put("sellClinetNickname", next->getNickname());
					}

					for (std::weak_ptr<ClientSide> c: room->getClientSideList())
					{
						if (c.expired())
						{
							LOG_WARN << "Client expired while broadcasting showPokers, skipping";
							continue;
						}
						std::shared_ptr<ClientSide> client(c.lock());
						if (client->getRole() == ClientRole::PLAYER)
						{
							LOG_DEBUG << "我是服务器，我想要让玩家的客户端显示当前打的牌, 当前的客户端id： "
									  << client->getId();
							pushDataToClient(codec,
											 client->getConn(),
											 ClientEventCode::CODE_SHOW_POKERS,
											 mapHelper);
						}
					}

					// notifyWatcherPlayPoker(room, result);
//					LOG_WARN << "notifyWatcherPlayPoker(room, result);";
					if (clientSide->getPokers().empty())
					{
						LOG_DEBUG << "游戏结束！";
						MapHelper result;
						std::string winnerType = int(clientSide->getType()) ? "PEASANT" : "LANDLORD";
						result.put("winnerNickname", clientSide->getNickname())
							  .put("winnerType", winnerType);
						for (std::weak_ptr<ClientSide> c: room->getClientSideList())
						{
							if (c.expired())
							{
								LOG_WARN << "Client expired while broadcasting gameOver, skipping";
								continue;
							}
							std::shared_ptr<ClientSide> client(c.lock());
							if (client->getRole() == ClientRole::PLAYER)
							{
								LOG_DEBUG << "client->getRole() == ClientRole::PLAYER";
								pushDataToClient(codec,
										         client->getConn(),
												 ClientEventCode::CODE_GAME_OVER,
												 result);
							}
						}

						ServerEventListener_CODE_CLIENT_EXIT(codec, conn, MapHelper());
					}
					else
					{
						if (next->getRole() == ClientRole::PLAYER)
						{
							mapHelper.setValue("clientId", next->getId());
							LOG_DEBUG << "修改测试： " << mapHelper.get("clientId", 0);
							LOG_DEBUG << "玩家ID：" << next->getId()  <<  "玩家整理牌！";
							ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT(codec,
																			  next->getConn(),
																			  mapHelper);
						}
						else
						{
							LOG_DEBUG << "轮到机器人瞎集二打了";
							LOG_DEBUG << "机器人的名字是： " << next->getNickname();
							std::thread robot_task(RobotEventListener::get,
												   codec,
									               std::ref(conn),
									               ClientEventCode::CODE_GAME_POKER_PLAY,
									               next.get(),
												   mapHelper);

							robot_task.join();
						}
					}
				}  // 141: if (sellFlag)
				else
				{
					pushDataToClient(codec, conn,
									 ClientEventCode::CODE_GAME_POKER_PLAY_INVALID,
									 MapHelper());
				}
			}   // 87: checkIndex
		}  // 72
		else
		{
			pushDataToClient(codec, conn,
							 ClientEventCode::CODE_GAME_POKER_PLAY_ORDER_ERROR,
							 MapHelper());
		}
	}
	else
	{
		// ChannelUtils.pushToClient(clientSide.getChannel(), ClientEventCode.CODE_ROOM_PLAY_FAIL_BY_INEXIST, null);
		LOG_WARN << "ChannelUtils.pushToClient(clientSide.getChannel(), ClientEventCode.CODE_ROOM_PLAY_FAIL_BY_INEXIST, null);";
	}
}


void ServerEventListener_CODE_GAME_STARTING(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											const MapHelper &data)
{
	int clientId = data.get("clientId", 0);
	assert(clientId != 0);
//	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.at(clientId);
	std::shared_ptr<ClientSide> clientSide = ServerContains::getClient(clientId);
	LOG_DEBUG << "clientId: " << clientId;
	if (clientId < 0)
	{
		LOG_DEBUG << "我是机器人！我要继续选地主了";
	}
	else
	{
		LOG_DEBUG << "我是玩家，我要打牌了！";
	}
//	Room *room = ServerContains::getRoom(clientSide->getRoomId());
	std::shared_ptr<Room> room = ServerContains::getRoom(clientSide->getRoomId());

	assert( room != NULL);
	assert(ServerContains::CLIENT_SIDE_MAP.at(clientId) != NULL);

	std::vector<std::weak_ptr<ClientSide> > roomClientList = room->getClientSideList();
	LOG_DEBUG << "roomClientList::size(): " << roomClientList.size() << "\n";

	// Send the points of poker
	std::vector<std::vector<Poker> > pokersList = PokerHelper::distributePoker();
	LOG_INFO << "pokersList.size(): " << pokersList.size() << "\n";
	int cursor = 0;
	for (std::weak_ptr<ClientSide> c: roomClientList)
	{
		if (c.expired())
			throw std::runtime_error("ClientID 不存在");
		std::shared_ptr<ClientSide> client(c.lock());
		client->setPokers(pokersList.at(cursor++));
	}
	room->setLandlordPokers(pokersList.at(3));   // 设置底牌

	LOG_INFO << "print pokers: \n";
	for (std::weak_ptr<ClientSide> c: roomClientList)
	{
		if (c.expired())
			throw std::runtime_error("ClientID 不存在");
		std::shared_ptr<ClientSide> client(c.lock());
		LOG_INFO << PokerHelper::printPokers(client->getPokers());
	}

	// Push information about the robber
	int startGrabIndex = rand() % 3;   // 随机找个人开始选地主
	std::weak_ptr<ClientSide> sgc = roomClientList.at(startGrabIndex);
	// 如果随机选中的客户端已过期（如机器人断开），顺延找一个可用的
	for (int i = 0; i < (int)roomClientList.size() && sgc.expired(); ++i)
	{
		startGrabIndex = (startGrabIndex + 1) % roomClientList.size();
		sgc = roomClientList.at(startGrabIndex);
	}
	if (sgc.expired())
	{
		LOG_ERROR << "No available client to start landlord elect";
		return;
	}
	std::shared_ptr<ClientSide> startGrabClient = sgc.lock();
	room->setCurrentSellClient(startGrabClient->getId());

//	// Push start game message
	room->setStatus(RoomStatus::STARTING);

	// Record the first speaker
	room->setFirstSellClient(startGrabClient->getId());

	for (std::weak_ptr<ClientSide> c: roomClientList)
	{
		if (c.expired())
		{
			LOG_WARN << "Client expired while broadcasting gameStarting, skipping";
			continue;
		}
		std::shared_ptr<ClientSide> client(c.lock());
		client->setType(ClientType::PEASANT);   // 农民

		MapHelper result;
		result.put("clientId", client->getId())
			  .put("roomId", room->getId())
			  .put("roomOwner", room->getRoomOwner())
			  .put("roomClientCount", room->getClientSideList().size())
			  .put("nextClientNickname", startGrabClient->getNickname())
			  .put("nextClientId", startGrabClient->getId())
			  .put("pokers", client->getPokers());

		LOG_INFO << "ClientRole: " << int(clientSide->getRole());
		if (client->getRole() == ClientRole::PLAYER)  // 玩家
		{
			LOG_DEBUG << "客户端 ID: " << client->getId();
			pushDataToClient(codec,
							 client->getConn(),
							 ClientEventCode::CODE_GAME_STARTING,
							 result);
		}
		else  // 人机
		{

			LOG_INFO << "继续选地主！";
			if(startGrabClient->getId() == client->getId())
			{
				std::thread robot_task(RobotEventListener::get,
									   codec,
						               std::ref(conn),
						               ClientEventCode::CODE_GAME_LANDLORD_ELECT,
						               client.get(),
						               result);

				robot_task.join();
			}
		}
	}
	// notifyWatcherGameStart(room);
	LOG_WARN << "notifyWatcherGameStart(room);";
}

void ServerEventListener_CODE_ROOM_CREATE_PVE(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  const MapHelper &data)
{
	LOG_INFO << "ServerEventListener_CODE_ROOM_CREATE_PVE";
	int clientId = data.get("clientId", 0);
	int difficultyCoefficient = atoi(data.get("choose", "").c_str());
	LOG_DEBUG << "difficultyCoefficient: " << difficultyCoefficient;
//	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.find(clientId)->second;
	std::weak_ptr<ClientSide> cs = ServerContains::getClient(clientId);
	if (cs.expired())
		throw std::runtime_error("ClientId 不存在");
	std::shared_ptr<ClientSide> clientSide = cs.lock();
	assert(clientId != 0);
	LOG_INFO << "clientId: " << clientId << "\n";
	LOG_INFO << "clientSide.id: " << clientSide->getId() << "\n";
	if (!ServerContains::hasRobot(2))
	{
		LOG_WARN << "Not enough external robots in pool";
		pushDataToClient(codec, conn,
						 ClientEventCode::CODE_PVE_DIFFICULTY_NOT_SUPPORT,
						 MapHelper());
		return;
	}

	LOG_INFO << "clientId: " << clientId << "\n";
	LOG_INFO << "clientSide.id: " << clientSide->getId() << "\n";

	std::shared_ptr<Room> room(new Room(ServerContains::getServerId()));
	LOG_INFO << "room create";
	room->setType(RoomType::PVE);
	room->setStatus(RoomStatus::BLANK);
	room->setRoomOwner(clientSide->getNickname());
	room->getClientSideMap().insert(std::make_pair(clientSide->getId(), clientSide));
	room->getClientSideList().push_back(clientSide);
	room->setCurrentSellClient(clientSide->getId());
	room->setDifficultyCoefficient(difficultyCoefficient);

	LOG_INFO << "room init complete";

	clientSide->setRoomId(room->getId());
	ServerContains::addRoom(room);
	std::shared_ptr<ClientSide> preClient = clientSide;
	for (int index = 1; index < 3; ++index)
	{
		LOG_INFO << index << "\n";
		std::shared_ptr<ClientSide> robot = ServerContains::takeRobot();
		if (!robot)
			throw std::runtime_error("Robot pool unexpectedly empty");

		robot->setRoomId(room->getId());
		robot->setStatus(ClientStatus::PLAYING);
		robot->setPre(preClient);
		preClient->setNext(robot);
		LOG_INFO << "robot->getId(): " << robot->getId();
		room->getClientSideMap().insert(std::make_pair(robot->getId(), robot));
		room->getClientSideList().push_back(robot);
		preClient = robot;
	}

	preClient->setNext(clientSide);
	clientSide->setPre(preClient);
	LOG_DEBUG << "查看 CLIENT_SIDE_MAP 里存的 clientSide：";
	for (auto & client: ServerContains::CLIENT_SIDE_MAP)
	{
		LOG_DEBUG << "client->id: " << client.second->getId();
	}

	LOG_DEBUG << "查看room中的 clientSide 信息：";
	for (auto client: room->getClientSideList())
	{
		if (!client.expired())
			LOG_DEBUG << "client.id of room: " << client.lock()->getId();
	}

	LOG_INFO << "ServerEventCode::CODE_GAME_STARTING";
	ServerEventListener_CODE_GAME_STARTING(codec, conn,
												   MapHelper().put("roomId", room->getId())
																  .put("clientId", clientSide->getId()));
	LOG_INFO << "ServerEventCode::CODE_GAME_STARTING";
}

void ServerEventListener_CODE_GAME_LANDLORD_ELECT(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												  const MapHelper &data)
{
	// 谁先抢，谁就是地主
	LOG_DEBUG << "ServerEventListener_CODE_GAME_LANDLORD_ELECT";
	int clientId = data.get("clientId", 0);
//	ClientSide &clientSide = *(ServerContains::CLIENT_SIDE_MAP.find(clientId)->second);

	std::weak_ptr<ClientSide> cs = ServerContains::getClient(clientId);
	if (cs.expired())
		throw std::runtime_error("ClientId 不存在");
	std::shared_ptr<ClientSide> clientSide = cs.lock();

	assert(ServerContains::CLIENT_SIDE_MAP.find(clientId)->second != NULL);
	LOG_INFO << "clientId: " << clientId;
	assert(clientId != 0);
	std::shared_ptr<Room> room = ServerContains::getRoom(clientSide->getRoomId());
	assert(ServerContains::getRoom(clientSide->getRoomId()) != NULL);

	LOG_INFO << "roomId: " << room->getId();
	LOG_DEBUG << "is_Y: " <<  data.get("is_Y", "");

	if (data.get("is_Y", "") == "true")
	{
		LOG_INFO << "true";
		std::copy(room->getLoadlordPokers()->begin(),
				  room->getLoadlordPokers()->end(),
				  std::back_inserter(clientSide->getPokers()));

		PokerHelper::sortPoker(clientSide->getPokers());

		clientSide->setType(ClientType::LANDLORD);

		int currentClientId = clientSide->getId();
		room->setLandlordId(currentClientId);
		room->setLastSellClient(currentClientId);
		room->setCurrentSellClient(currentClientId);

		for (std::weak_ptr<ClientSide> c: room->getClientSideList())
		{
			if (c.expired())
			{
				LOG_WARN << "Client expired while broadcasting landlordConfirm, skipping";
				continue;
			}
			std::shared_ptr<ClientSide> client = c.lock();
			MapHelper result;
			result.put("roomId", room->getId())
				  .put("roomOwner", room->getRoomOwner())
				  .put("roomClientCount", room->getClientSideList().size())
				  .put("landlordNickname", clientSide->getNickname())
				  .put("landlordId", clientSide->getId())
				  .put("additionalPokers", room->getLandlordPokers());

			if (client->getRole() == ClientRole::PLAYER)
			{
				LOG_INFO << "我是玩家，终于可以玩牌啦！";
				pushDataToClient(codec,
						         client->getConn(),
								 ClientEventCode::CODE_GAME_LANDLORD_CONFIRM,
								 result);
			}
			else
			{
				if(currentClientId == client->getId())
				{
					LOG_INFO << "地主确认，机器人开始营业啦！";
					std::thread robot_task(RobotEventListener::get,
										   codec,
							               std::ref(conn),
							               ClientEventCode::CODE_GAME_POKER_PLAY,
							               client.get(),
							               result);

					robot_task.join();
				}
			}
		}

		// notifyWatcherRobLandlord(room, clientSide);
		LOG_WARN << "notifyWatcherRobLandlord(room, clientSide)";
	}
	else
	{
		LOG_DEBUG << "不要地主";
		if (clientSide->getNext()->getId() == room->getFirstClient())
		{
			LOG_INFO << "id: " << clientSide->getId();
			for (std::weak_ptr<ClientSide> client: room->getClientSideList())
			{
				if (client.expired())
				{
					LOG_WARN << "Client expired while broadcasting landlordCycle, skipping";
					continue;
				}
				if (client.lock()->getRole() == ClientRole::PLAYER)
				{
					pushDataToClient(codec,
							         client.lock()->getConn(),
									 ClientEventCode::CODE_GAME_LANDLORD_CYCLE,
									 MapHelper());
				}
			}

			ServerEventListener_CODE_GAME_STARTING(codec, conn,
												   MapHelper().put("clientId", clientId));
		}
		else
		{
			LOG_INFO << "id: " << clientSide->getId();
			std::shared_ptr<ClientSide> turnClientSide = clientSide->getNext();
			room->setCurrentSellClient(turnClientSide->getId());
			MapHelper result;
			result.put("roomId", room->getId())
				  .put("roomOwner", room->getRoomOwner())
				  .put("roomClientCount", room->getClientSideList().size())
				  .put("preClientNickname", clientSide->getNickname())
				  .put("nextClientNickname", turnClientSide->getNickname())
				  .put("nextClientId", turnClientSide->getId());

			for (std::weak_ptr<ClientSide> c: room->getClientSideList())
			{
				if (c.expired())
				{
					LOG_WARN << "Client expired while broadcasting landlordElect, skipping";
					continue;
				}
				std::shared_ptr<ClientSide> client = c.lock();
				if (client->getRole() == ClientRole::PLAYER)
				{
					pushDataToClient(codec,
									 client->getConn(),
									 ClientEventCode::CODE_GAME_LANDLORD_ELECT,
									 result);
				}
				else
				{
					LOG_INFO << "robot: " << "comming in";
					LOG_INFO << "robot_" << client->getId();
					if (client->getId() == turnClientSide->getId())
					{
						LOG_DEBUG << "client->getId(): " << client->getId();
						LOG_DEBUG << "turnClientSide.getId(): " << turnClientSide->getId();
						std::thread robot(std::bind(robot_elect_landlord, codec, conn,
										  ClientEventCode::CODE_GAME_LANDLORD_ELECT,
										  client.get(), result));
						robot.join();
					}
				}
			}
			// notifyWatcherRobLandlord(room, clientSide);
			LOG_WARN << "notifyWatcherRobLandlord(room, clientSide)";
		}
	}
}
//	else
//	{
//		// ChannelUtils.pushToClient(clientSide.getChannel(), ClientEventCode.CODE_ROOM_PLAY_FAIL_BY_INEXIST, null);
//		LOG_WARN << "ChannelUtils.pushToClient(clientSide.getChannel(), ClientEventCode.CODE_ROOM_PLAY_FAIL_BY_INEXIST, null)";
//	}

void ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT(WsCodec *codec, const muduo::net::TcpConnectionPtr &conn,
													   const MapHelper &data)
{
	LOG_INFO << "我是服务端，我在做出牌前的准备！";
	int clientId = data.get("clientId", 0);
	LOG_DEBUG << "客户端ID: " << clientId;
//	assert(clientId > 0);
//	ClientSide &clientSide = *(ServerContains::CLIENT_SIDE_MAP.find(clientId)->second);
	std::weak_ptr<ClientSide> cs = ServerContains::getClient(clientId);
	if (cs.expired())
	{
		LOG_WARN << "Target client expired in playRedirect, clientId=" << clientId;
		return;
	}
	std::shared_ptr<ClientSide> clientSide = cs.lock();
	LOG_INFO << "clientId: " << clientId;
	// FIXME
	std::shared_ptr<Room> room = ServerContains::getRoom(clientSide->getRoomId());
	assert(ServerContains::getRoom(clientSide->getRoomId()) != NULL);
	assert(ServerContains::CLIENT_SIDE_MAP.at(clientId) != NULL);

	LOG_INFO << "clientSide->getId(): " << clientSide->getId();
//	LOG_INFO << "clientSide->getPre().getId(): " << clientSide->getPre()->getId();

	std::vector<ClientInfo> clientInfos;
	for (std::weak_ptr<ClientSide> c: room->getClientSideList())
	{
		if (c.expired())
		{
			LOG_WARN << "Client expired while building clientInfos, skipping";
			continue;
		}
		std::shared_ptr<ClientSide> client = c.lock();
		std::string p;
		if (clientSide->getPre() != nullptr)
			p = clientSide->getPre()->getId() == client->getId() ? "up" : "down";
		MapHelper result;

		ClientInfo info(client->getId(),
						 client->getNickname(),
						 client->getType(),
						 client->getPokers().size(),
						 p);

		if (client->getId() != clientSide->getId())
			clientInfos.push_back(info);
	}

	ClientInfo cinfo(clientSide->getId(),
					 clientSide->getNickname(),
					 clientSide->getType(),
					 clientSide->getPokers().size(),
					 "");

	clientInfos.push_back(cinfo);

	LOG_DEBUG << "clientInfos 的大小： " << clientInfos.size();

	MapHelper result;
	result.put("pokers", clientSide->getPokers())
		  .put("lastSellPokers", data.get("lastSellPokers", std::vector<Poker>()))
		  .put("lastSellClientId", data.get("lastSellClientId", 0))
		  .put("clientInfos", clientInfos)
		  .put("sellClientId", room->getCurrentSellClient())
		  .put("sellClinetNickname", ServerContains::CLIENT_SIDE_MAP.at(room->getCurrentSellClient())->getNickname());

	pushDataToClient(codec, conn,
					 ClientEventCode::CODE_GAME_POKER_PLAY_REDIRECT,
					 result);
}

void robot_elect_landlord(WsCodec *codec,
							const muduo::net::TcpConnectionPtr &conn,
							ClientEventCode code,
							ClientSide *robot,
							const MapHelper &mapHelper)
{
	RobotEventListener::get(codec, conn,
							code,
							robot, mapHelper);
}

