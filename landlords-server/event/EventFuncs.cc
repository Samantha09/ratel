/*
 * EventFuncs.cc
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */


#include "EventFuncs.h"
#include "robot/RobotDecisionMakers.h"
#include "ServerContains.h"
#include "protobuf/query.pb.h"
#include "enums/ClientEventCode.h"
#include "helper/PokerHelper.h"
#include "muduo/base/Logging.h"
#include "helper/SerializeHelper.h"
#include "../robot/RobotEventListener.h"
#include <thread>

void robot_elect_landlord(ProtobufCodec *codec,
							const muduo::net::TcpConnectionPtr &conn,
							ClientEventCode code,
							ClientSide *robot,
							const MapHelper &mapHelper);

void pushDataToClient(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ClientEventCode code, const MapHelper &mapHelper)
{
//	  LOG_DEBUG << "pushDataToClient " << int(code);
	  LOG_DEBUG << clientEventCodeToString[int(code)];
	  std::string result = SerializeHelper::SerializeToString<MapHelper>(mapHelper);
	  muduo::Answer answer;
	  answer.set_answerer("san");
	  answer.set_questioner("san");
	  answer.set_id(int(code));
	  answer.add_solution(result);
	  codec->send(conn, answer);
}

void ServerEventListener_CODE_CLIENT_EXIT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
										  const MapHelper &data)
{
	LOG_DEBUG << "ServerEventListener_CODE_CLIENT_EXIT";
	conn->shutdown();
}

void ServerEventListener_CODE_CLIENT_NICKNAME_SET(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												  const MapHelper &data)
{
	LOG_INFO << "ServerEventListener_CODE_CLIENT_NICKNAME_SET \n";
//	ClientTransferData result = SerializeHelper::parseStringToData<ClientTransferData>(data);
	int clientId = data.get("clientId", 0);
//	assert(clientId != 0);
	std::string nickname = data.get("nickName", "");
	assert(!nickname.empty());

	if (clientId != 0 && nickname != "")
	{
		ServerContains::CLIENT_SIDE_MAP.at(clientId)->setNickname(nickname);
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

void ServerEventListener_CODE_GAME_POKER_PLAY_PASS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												   const MapHelper &data)
{
	LOG_DEBUG << "ServerEventListener_CODE_GAME_POKER_PLAY_PASS";
	int clientId = data.get("clientId", 0);
	assert(clientId != 0);
	ClientSide &clientSide = *(ServerContains::CLIENT_SIDE_MAP.at(clientId));
	std::string type = clientSide.type_ ? "农民" : "地主";
	LOG_DEBUG << "clientId: " << clientId << "type: " << type;
	LOG_DEBUG << "clientSide.getRoomId(): " << clientSide.getRoomId();
	Room &room = *ServerContains::getRoom(clientSide.getRoomId());
	assert(ServerContains::getRoom(clientSide.getRoomId()) != NULL);
	assert(ServerContains::CLIENT_SIDE_MAP.at(clientId) != NULL);
	LOG_DEBUG << "room.id: " << room.getId();
	if (room.getCurrentSellClient() == clientSide.getId())
	{
		if (clientSide.getId() != room.getLastSellClient())
		{
			ClientSide turnClient = clientSide.getNext();
			room.setCurrentSellClient(turnClient.getId());

			for (ClientSide *client: room.getClientSideList())
			{
				MapHelper result;
				result.put("clientId", clientSide.getId())
					  .put("clientNickname", clientSide.getNickname())
					  .put("nextClientId", turnClient.getId())
					  .put("nextClientNickname", turnClient.getNickname());


				if (client->getRole() == ClientRole::PLAYER)
				{
					pushDataToClient(codec, conn,
									 ClientEventCode::CODE_GAME_POKER_PLAY_PASS,
									 result);
				}
				else
				{
					if (client->getId() == turnClient.getId())
					{
						LOG_DEBUG << "pass 里的逻辑";
						std::thread robot_task(RobotEventListener::get,
											   codec,
								               std::ref(conn),
								               ClientEventCode::CODE_GAME_POKER_PLAY,
								               &turnClient,
								               data);
						robot_task.join();
					}
				}
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

void ServerEventListener_CODE_GAME_POKER_PLAY(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  const MapHelper &data)
{
//	LOG_DEBUG << "ServerEventListener_CODE_GAME_POKER_PLAY";
	int clientId = data.get("clientId", 0);
	assert(clientId != 0);
	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.at(clientId);
//	LOG_INFO << "clientId: " << clientId;

	if (clientId < 0)
		LOG_DEBUG << "我是服务器，现在收到了机器人的出牌申请！";
	else
		LOG_DEBUG << "我是服务器，现在收到了玩家的出牌申请！";
	Room *room = ServerContains::getRoom(clientSide->getRoomId());

	assert(room != NULL);
	assert(ServerContains::CLIENT_SIDE_MAP.at(clientId) != NULL);

	if (room != NULL)
	{
		if (room->getCurrentSellClient() == clientSide->getId())
		{
			LOG_WARN << "\n currentSellClient: " << room->getCurrentSellClient()
					 << "clientSide.id: " << clientSide->getId();
			std::vector<PokerLevel> options = data.get("options", std::vector<PokerLevel>());

			std::vector<int> indexes = PokerHelper::getIndexes(options, clientSide->getPokers());

			if (PokerHelper::checkPokerIndex(indexes, clientSide->getPokers()))
			{
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
					ClientSide &next = clientSide->getNext();
					room->setLastSellClient(clientSide->getId());
					room->setCurrentSellClient(next.getId());

					LOG_INFO << "clientSide->getPokers().size(): " << clientSide->getPokers().size();
					PokerHelper::removeAll(clientSide->getPokers(), currentPokers);
					LOG_INFO << "clientSide->getPokers().size(): " << clientSide->getPokers().size();

					MapHelper mapHelper;
					mapHelper.put("clientId", clientSide->getId())
							 .put("clientNickname",clientSide->getNickname())
							 .put("clientType", int(clientSide->getType()))
							 .put("pokers", currentPokers)
							 .put("lastSellClientId", clientSide->getId())
							 .put("lastSellPokers", currentPokers);

					if (!clientSide->getPokers().empty())
					{
						mapHelper.put("sellClinetNickname", next.getNickname());
					}

					for (ClientSide *client: room->getClientSideList())
					{
						if (client->getRole() == ClientRole::PLAYER)
						{
							LOG_DEBUG << "我是服务器，我想要让玩家的客户端显示当前打的牌, 当前的客户端id： "
									  << client->getId();
							pushDataToClient(codec, conn,
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
						result.put("winnerNickname", clientSide->getNickname())
							  .put("winnerType", int(clientSide->getType()));
						for (ClientSide *client: room->getClientSideList())
						{
							if (client->getRole() == ClientRole::PLAYER)
							{
								LOG_DEBUG << "client->getRole() == ClientRole::PLAYER";
								pushDataToClient(codec, conn,
												 ClientEventCode::CODE_GAME_OVER,
												 result);
							}
						}

						ServerEventListener_CODE_CLIENT_EXIT(codec, conn, MapHelper());
					}
					else
					{
						if (next.getRole() == ClientRole::PLAYER)
						{
							LOG_DEBUG << "玩家整理牌！";
							ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT(codec,
																			  conn,
																			  mapHelper);
						}
						else
						{
							LOG_DEBUG << "轮到机器人瞎集二打了";
							LOG_DEBUG << "机器人的名字是： " << next.getNickname();
							std::thread robot_task(RobotEventListener::get,
												   codec,
									               std::ref(conn),
									               ClientEventCode::CODE_GAME_POKER_PLAY,
									               &next,
									               data);

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


void ServerEventListener_CODE_GAME_STARTING(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											const MapHelper &data)
{
	int clientId = data.get("clientId", 0);
	assert(clientId != 0);
	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.at(clientId);
	LOG_DEBUG << "clientId: " << clientId;
	if (clientId < 0)
	{
		LOG_DEBUG << "我是机器人！我要继续选地主了";
	}
	else
	{
		LOG_DEBUG << "我是玩家，我要打牌了！";
	}
	Room *room = ServerContains::getRoom(clientSide->getRoomId());

	assert( room != NULL);
	assert(ServerContains::CLIENT_SIDE_MAP.at(clientId) != NULL);

	std::vector<ClientSide*> roomClientList = room->getClientSideList();
	LOG_DEBUG << "roomClientList::size(): " << roomClientList.size() << "\n";

	std::vector<ClientSide> roomCList;

	// Send the points of poker
	std::vector<std::vector<Poker> > pokersList = PokerHelper::distributePoker();
	LOG_INFO << "pokersList.size(): " << pokersList.size() << "\n";
	int cursor = 0;
	for (ClientSide *client: roomClientList)
	{
		client->setPokers(pokersList.at(cursor++));
	}
	room->setLandlordPokers(pokersList.at(3));   // 设置底牌

	LOG_INFO << "print pokers: \n";
	for (ClientSide *client: roomClientList)
	{
		LOG_INFO << PokerHelper::printPokers(client->getPokers());
	}

	// Push information about the robber
	int startGrabIndex = rand() % 3;   // 随机找个人开始选地主
	ClientSide *startGrabClient = roomClientList.at(startGrabIndex);
	room->setCurrentSellClient(startGrabClient->getId());

//	// Push start game message
	room->setStatus(RoomStatus::STARTING);

	// Record the first speaker
	room->setFirstSellClient(startGrabClient->getId());

	for (ClientSide *client: roomClientList)
	{
		client->setType(ClientType::PEASANT);   // 农民

		MapHelper result;
		result.put("roomId", room->getId())
			  .put("roomOwner", room->getRoomOwner())
			  .put("roomClientCount", room->getClientSideList().size())
			  .put("nextClientNickname", startGrabClient->getNickname())
			  .put("nextClientId", startGrabClient->getId())
			  .put("pokers", client->getPokers())
			  .put("clientOrderList", roomCList);

		LOG_INFO << "ClientRole: " << int(clientSide->getRole());
		if (client->getRole() == ClientRole::PLAYER)  // 玩家
		{
			pushDataToClient(codec, conn,
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
						               client,
						               result);

				robot_task.join();
			}
		}
	}
	// notifyWatcherGameStart(room);
	LOG_WARN << "notifyWatcherGameStart(room);";
}

void ServerEventListener_CODE_ROOM_CREATE_PVE(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
											  const MapHelper &data)
{
	LOG_INFO << "ServerEventListener_CODE_ROOM_CREATE_PVE";
	int clientId = data.get("clientId", 0);
	int difficultyCoefficient = atoi(data.get("choose", "").c_str());
	LOG_DEBUG << "difficultyCoefficient: " << difficultyCoefficient;
	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.find(clientId)->second;
	assert(clientId != 0);
	LOG_INFO << "clientId: " << clientId << "\n";
	LOG_INFO << "clientSide.id: " << clientSide->getId() << "\n";
	if (RobotDecisionMakers::contains(difficultyCoefficient))
	{
		LOG_INFO << "contains(difficultyCoefficient)";
		Room *room(new Room(ServerContains::getServerId()));
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
		// 不要返回局部对象的引用
		ServerContains::addRoom(room);

		ClientSide *preClient = clientSide;
		// Add robot;
		for (int index = 1; index < 3; ++index)
		{
			LOG_INFO << index << "\n";
			ClientSide *robot(new ClientSide(- ServerContains::getClientId(), ClientStatus::PLAYING));
			// FIXME:
//			std::string name = "robot_" + std::to_string(index);
			robot->setNickname("robot_" + std::to_string(index));
			robot->setRole(ClientRole::ROBOT);
			preClient->setNext(robot);
			robot->setPre(preClient);
			robot->setRoomId(room->getId());
			LOG_INFO << "robot->getRole(): " << int(robot->getRole());
			room->getClientSideMap().insert(std::make_pair(robot->getId(), robot));
			room->getClientSideList().push_back(robot);

			preClient = robot;
			ServerContains::CLIENT_SIDE_MAP.insert(std::make_pair(robot->getId(), robot));
			LOG_INFO << "ClientSideList.size(): " << room->getClientSideList().size() << "\n";
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
			LOG_DEBUG << "client.id of room: " << client->getId();
		}

		LOG_INFO << "ServerEventCode::CODE_GAME_STARTING";
		ServerEventListener_CODE_GAME_STARTING(codec, conn,
											   MapHelper().put("roomId", room->getId())
											   	   	   	  .put("clientId", clientSide->getId()));
		LOG_INFO << "ServerEventCode::CODE_GAME_STARTING";
	}
	else
	{
		pushDataToClient(codec, conn,
				         ClientEventCode::CODE_PVE_DIFFICULTY_NOT_SUPPORT,
						 MapHelper());
	}
}

void ServerEventListener_CODE_GAME_LANDLORD_ELECT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
												  const MapHelper &data)
{
	// 谁先抢，谁就是地主
	LOG_DEBUG << "ServerEventListener_CODE_GAME_LANDLORD_ELECT";
//	ClientTransferData result = SerializeHelper::parseStringToData<ClientTransferData>(data);
	int clientId = data.get("clientId", 0);
	ClientSide &clientSide = *(ServerContains::CLIENT_SIDE_MAP.find(clientId)->second);

	assert(ServerContains::CLIENT_SIDE_MAP.find(clientId)->second != NULL);
	LOG_INFO << "clientId: " << clientId;
	assert(clientId != 0);
	Room &room = *ServerContains::getRoom(clientSide.getRoomId());
	assert(ServerContains::getRoom(clientSide.getRoomId()) != NULL);

	LOG_INFO << "roomId: " << room.getId();
	LOG_DEBUG << "is_Y: " <<  data.get("is_Y", "");

	if (data.get("is_Y", "") == "true")
	{
		LOG_INFO << "true";
		std::copy(room.getLoadlordPokers()->begin(),
				  room.getLoadlordPokers()->end(),
				  std::back_inserter(clientSide.getPokers()));

		PokerHelper::sortPoker(clientSide.getPokers());

		int currentClientId = clientSide.getId();
		room.setLandlordId(currentClientId);
		room.setLastSellClient(currentClientId);
		room.setCurrentSellClient(currentClientId);

		for (ClientSide *client: room.getClientSideList())
		{
			MapHelper result;
			result.put("roomId", room.getId())
				  .put("roomOwner", room.getRoomOwner())
				  .put("roomClientCount", room.getClientSideList().size())
				  .put("landlordNickname", clientSide.getNickname())
				  .put("landlordId", clientSide.getId())
				  .put("additionalPokers", room.getLandlordPokers());

			if (client->getRole() == ClientRole::PLAYER)
			{
				LOG_INFO << "我是玩家，终于可以玩牌啦！";
				pushDataToClient(codec, conn,
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
							               client,
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
		if (clientSide.getNext().getId() == room.getFirstClient())
		{
			LOG_INFO << "id: " << clientSide.getId();
			for (ClientSide *client: room.getClientSideList())
			{
				if (client->getRole() == ClientRole::PLAYER)
				{
					pushDataToClient(codec, conn,
									 ClientEventCode::CODE_GAME_LANDLORD_CYCLE,
									 MapHelper());
				}
			}

			ServerEventListener_CODE_GAME_STARTING(codec, conn,
												   MapHelper().put("clienId", clientId));
		}
		else
		{
			LOG_INFO << "id: " << clientSide.getId();
			ClientSide &turnClientSide = clientSide.getNext();
			room.setCurrentSellClient(turnClientSide.getId());
			MapHelper result;
			result.put("roomId", room.getId())
				  .put("roomOwner", room.getRoomOwner())
				  .put("roomClientCount", room.getClientSideList().size())
				  .put("preClientNickname", clientSide.getNickname())
				  .put("nextClientNickname", turnClientSide.getNickname())
				  .put("nextClientId", turnClientSide.getId());

			for (ClientSide *client: room.getClientSideList())
			{
				if (client->getRole() == ClientRole::PLAYER)
				{
					pushDataToClient(codec, conn,
									 ClientEventCode::CODE_GAME_LANDLORD_ELECT,
									 result);
				}
				else
				{
					LOG_INFO << "robot: " << "comming in";
					LOG_INFO << "robot_" << client->getId();
					if (client->getId() == turnClientSide.getId())
					{
						LOG_DEBUG << "client->getId(): " << client->getId();
						LOG_DEBUG << "turnClientSide.getId(): " << turnClientSide.getId();
						std::thread robot(std::bind(robot_elect_landlord, codec, conn,
										  ClientEventCode::CODE_GAME_LANDLORD_ELECT,
										  client, result));
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

void ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
													   const MapHelper &data)
{
	LOG_INFO << "我是服务端，我在做出牌前的准备！";
	int clientId = data.get("clientId", 0);
	assert(clientId != 0);
	ClientSide &clientSide = *(ServerContains::CLIENT_SIDE_MAP.find(clientId)->second);

	LOG_INFO << "clientId: " << clientId;
	// FIXME
	Room &room = *ServerContains::getRoom(clientSide.getRoomId());
	assert(ServerContains::getRoom(clientSide.getRoomId()) != NULL);
	assert(ServerContains::CLIENT_SIDE_MAP.at(clientId) != NULL);

	LOG_INFO << "clientSide->getId(): " << clientSide.getId();
	LOG_INFO << "clientSide->getPre().getId(): " << clientSide.getPre().getId();

	std::vector<ClientInfo> clientInfos;
	for (ClientSide *client: room.getClientSideList())
	{
		std::string p = clientSide.getPre().getId() == client->getId() ? "up" : "down";
		MapHelper result;

		ClientInfo info(client->getId(),
						 client->getNickname(),
						 client->getType(),
						 client->getPokers().size(),
						 p);

		if (client->getId() != clientSide.getId())
			clientInfos.push_back(info);
	}

	MapHelper result;
	result.put("pokers", clientSide.getPokers())
		  .put("lastSellPokers", data.get("lastSellPokers", std::vector<Poker>()))
		  .put("lastSellClientId", data.get("lastSellClientId", 0))
		  .put("clientInfos", clientInfos)
		  .put("sellClientId", room.getCurrentSellClient())
		  .put("sellClinetNickname", ServerContains::CLIENT_SIDE_MAP.at(room.getCurrentSellClient())->getNickname());

	pushDataToClient(codec, conn,
					 ClientEventCode::CODE_GAME_POKER_PLAY_REDIRECT,
					 result);
}

void robot_elect_landlord(ProtobufCodec *codec,
							const muduo::net::TcpConnectionPtr &conn,
							ClientEventCode code,
							ClientSide *robot,
							const MapHelper &mapHelper)
{
	RobotEventListener::get(codec, conn,
							code,
							robot, mapHelper);
}

