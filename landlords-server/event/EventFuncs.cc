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

void pushDataToClient(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ClientEventCode code, const std::string &data)
{
//	  LOG_DEBUG << "pushDataToClient " << int(code);
	  LOG_DEBUG << clientEventCodeToString[int(code)];
	  muduo::Answer answer;
	  answer.set_answerer("san");
	  answer.set_questioner("san");
	  answer.set_id(int(code));
	  answer.add_solution(data);
	  codec->send(conn, answer);
}

void ServerEventListener_CODE_CLIENT_EXIT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code, const std::string &data)
{
	conn->shutdown();
}

void ServerEventListener_CODE_CLIENT_NICKNAME_SET(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code, const std::string &data)
{
	LOG_INFO << "ServerEventListener_CODE_CLIENT_NICKNAME_SET \n";
	ClientTransferData result = SerializeHelper::parseStringToData<ClientTransferData>(data);
	ServerContains::CLIENT_SIDE_MAP.at(result.id)->setNickname(result.data);
	muduo::Answer answer;
	answer.set_answerer("san");
	answer.set_questioner("san");
	answer.set_id(int(ClientEventCode::CODE_SHOW_OPTIONS));
	answer.add_solution("");
	codec->send(conn, answer);
}

void ServerEventListener_CODE_GAME_POKER_PLAY_PASS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code, const std::string &data)
{
	LOG_DEBUG << "ServerEventListener_CODE_GAME_POKER_PLAY_PASS";
}

void ServerEventListener_CODE_GAME_POKER_PLAY(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code, const std::string &data)
{
	LOG_DEBUG << "ServerEventListener_CODE_GAME_POKER_PLAY";

//	CodeShowPokersData dataToTranfer;

	ClientTransferData result = SerializeHelper::parseStringToData<ClientTransferData>(data);
	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.at(result.id);
	LOG_INFO << "result.id: " << result.id;
	Room *room = ServerContains::getRoom(clientSide->getRoomId());

	if (room != NULL)
	{
		if (room->getCurrentSellClient() == clientSide->getId())
		{
			std::vector<PokerLevel> options = result.levels;

			for (auto pl: options)
			{
				LOG_INFO << "PokerLevel: " << int(pl);
			}

			LOG_INFO << "result.levels.size(): " << result.levels.size();
			LOG_INFO << "options.size(): " << options.size();
			std::vector<int> indexes = PokerHelper::getIndexes(options, clientSide->getPokers());

			LOG_DEBUG << "indexes.size(): " << indexes.size();

			if (PokerHelper::checkPokerIndex(indexes, clientSide->getPokers()))
			{
				LOG_DEBUG << "clientSide->id: " << clientSide->getId();
				bool sellFlag = true;

				std::vector<Poker> currentPokers = PokerHelper::getPoker(indexes, clientSide->getPokers());
				PokerSell currentPokerShell = PokerHelper::checkPokerType(currentPokers);
				if (currentPokerShell.getSellType() != SellType::ILLEGAL)
				{
					LOG_DEBUG << "currentPokerShell.getSellType(): " << currentPokerShell.getSellType();
					if (room->getLastSellClient() != clientSide->getId())
					{
						// FIXME: // room.getLastPokerShell() != null
						LOG_WARN << "FIXME: // room.getLastPokerShell() != null";
						PokerSell lastPokerShell = room->getLastPokerShell();

						if (lastPokerShell.getSellType() != currentPokerShell.getSellType() &&
							lastPokerShell.getSellPokers()->size() != currentPokerShell.getSellPokers()->size() &&
							currentPokerShell.getSellType() != SellType::BOMB &&
							currentPokerShell.getSellType() != SellType::KING_BOMB)
						{
							ClientGamePlayData dataToTransfer;
							dataToTransfer.setPlayType(currentPokerShell.getSellType());
							dataToTransfer.setPlayCount(currentPokerShell.getSellPokers()->size());
							dataToTransfer.setPreType(lastPokerShell.getSellType());
							dataToTransfer.setPreCount(lastPokerShell.getSellPokers()->size());
							std::string dataToTransferStr = SerializeHelper::SerializeToString<ClientGamePlayData>(dataToTransfer);

							sellFlag = false;
							pushDataToClient(codec, conn,
											 ClientEventCode::CODE_GAME_POKER_PLAY_MISMATCH,
											 dataToTransferStr);
						}    // 106
						else if (lastPokerShell.getScore() >= currentPokerShell.getScore())
						{
							ClientGamePlayData dataToTransfer;
							dataToTransfer.setPlayScore(currentPokerShell.getScore());
							dataToTransfer.setPreScore(lastPokerShell.getScore());
							std::string dataToTransferStr = SerializeHelper::SerializeToString<ClientGamePlayData>(dataToTransfer);
							sellFlag = false;
							pushDataToClient(codec, conn,
											 ClientEventCode::CODE_GAME_POKER_PLAY_LESS,
											 dataToTransferStr);
						}
					}
				}    // 93
				else
				{
					sellFlag = false;
					pushDataToClient(codec, conn,
									ClientEventCode::CODE_GAME_POKER_PLAY_INVALID,
									data);
				}

				if (sellFlag)
				{
					ClientSide &next = clientSide->getNext();
					room->setLastSellClient(clientSide->getId());
					room->setCurrentSellClient(next.getId());

					PokerHelper::removeAll(clientSide->getPokers(), currentPokers);
					CodeShowPokersData dataToTranfer;
					dataToTranfer.setClientId(clientSide->getId());
					dataToTranfer.setClientNickname(clientSide->getNickname());
					dataToTranfer.setClientType(clientSide->getType());
					dataToTranfer.setPokers(currentPokers);
					dataToTranfer.setlastSellClientId(clientSide->getId());
					dataToTranfer.setLastSellPokers(currentPokers);

					if (!clientSide->getPokers().empty())
					{
						dataToTranfer.setSellClinetNickname(next.getNickname());
					}

					std::string dataToTransferStr = SerializeHelper::SerializeToString<CodeShowPokersData>(dataToTranfer);

					for (ClientSide *client: room->getClientSideList())
					{
						if (client->getRole() == ClientRole::PLAYER)
						{
							LOG_DEBUG << "client->getRole() == ClientRole::PLAYER";
							pushDataToClient(codec, conn,
											 ClientEventCode::CODE_SHOW_POKERS,
											 dataToTransferStr);
						}
					}

					// notifyWatcherPlayPoker(room, result);
					LOG_WARN << "notifyWatcherPlayPoker(room, result);";
					if (clientSide->getPokers().empty())
					{
						ClientGamePlayData dataWinToTranfer;
						dataWinToTranfer.setWinnerNickname(clientSide->getNickname());
						dataWinToTranfer.setWinnerType(clientSide->getType());
						std::string dataWinToTranferStr = SerializeHelper::SerializeToString<ClientGamePlayData>(dataWinToTranfer);

						for (ClientSide *client: room->getClientSideList())
						{
							if (client->getRole() == ClientRole::PLAYER)
							{
								LOG_DEBUG << "client->getRole() == ClientRole::PLAYER";
								pushDataToClient(codec, conn,
												 ClientEventCode::CODE_GAME_OVER,
												 dataWinToTranferStr);
							}
						}

						ServerEventListener_CODE_CLIENT_EXIT(codec, conn, code, "");
					}
					else
					{
						if (next.getRole() == ClientRole::PLAYER)
						{
							LOG_DEBUG << "next->getRole() == ClientRole::PLAYER";
							ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT(codec, conn, code, dataToTransferStr);
						}
						else
						{
							LOG_DEBUG << "next->getRole() == ClientRole::ROBOT";
							RobotEventListener::get(codec, conn,
									ClientEventCode::CODE_GAME_POKER_PLAY,
									&next,
									data);
						}
					}
				}  // 141: if (sellFlag)
			}   // 87: checkIndex
			else
			{
				pushDataToClient(codec, conn,
								 ClientEventCode::CODE_GAME_POKER_PLAY_INVALID,
								 "");
			}
		}  // 72
		else
		{
			pushDataToClient(codec, conn,
							 ClientEventCode::CODE_GAME_POKER_PLAY_ORDER_ERROR,
							 "");
		}
	}
	else
	{
		// ChannelUtils.pushToClient(clientSide.getChannel(), ClientEventCode.CODE_ROOM_PLAY_FAIL_BY_INEXIST, null);
		LOG_WARN << "ChannelUtils.pushToClient(clientSide.getChannel(), ClientEventCode.CODE_ROOM_PLAY_FAIL_BY_INEXIST, null);";
	}
}


void ServerEventListener_CODE_GAME_STARTING(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code, const std::string &data)
{
	ClientTransferData result = SerializeHelper::parseStringToData<ClientTransferData>(data);
	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.at(result.id);
	LOG_INFO << "result.id: " << result.id << "\n";
	Room *room = ServerContains::getRoom(clientSide->getRoomId());

	std::vector<ClientSide*> roomClientList = room->getClientSideList();
	LOG_INFO << "roomClientList::size(): " << roomClientList.size() << "\n";

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
	int startGrabIndex = rand() % 3;   // 随机选地主
	ClientSide *startGrabClient = roomClientList.at(startGrabIndex);
	room->setCurrentSellClient(startGrabClient->getId());

//	// Push start game message
	room->setStatus(RoomStatus::STARTING);

	// Record the first speaker
	room->setFirstSellClient(startGrabClient->getId());

	for (ClientSide *client: roomClientList)
	{
		client->setType(ClientType::PEASANT);   // 农民

		ServerTransferData result;
		result.setRoomId(room->getId());
		result.setRoomOwner(room->getRoomOwner());
		result.setRoomClientCount(room->getClientSideList().size());
		result.setnextClientNickname(startGrabClient->getNickname());
		result.setNextClientId(startGrabClient->getId());
		result.setPokers(client->getPokers());

		// FIXME
		result.setClientOrderList(roomCList);

		LOG_INFO << "ClientRole: " << int(clientSide->getRole());
		std::string res = SerializeHelper::SerializeToString<ServerTransferData>(result);
		if (client->getRole() == ClientRole::PLAYER)  // 玩家
		{
			pushDataToClient(codec, conn,
					ClientEventCode::CODE_GAME_STARTING,
					res);
		}
		else  // 人机
		{

			LOG_INFO << "人机";
			if(startGrabClient->getId() == client->getId())
			{
				RobotEventListener::get(codec, conn,
						ClientEventCode::CODE_GAME_LANDLORD_ELECT,
						client,
						data);
			}
		}
	}
}

void ServerEventListener_CODE_ROOM_CREATE_PVE(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code,const std::string &data)
{
	LOG_INFO << "ServerEventListener_CODE_ROOM_CREATE_PVE";
	ClientTransferData result = SerializeHelper::parseStringToData<ClientTransferData>(data);
	int difficultyCoefficient = 1;
	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.find(result.id)->second;
	LOG_INFO << "result.id: " << result.id << "\n";
	LOG_INFO << "clientSide.id: " << clientSide->getId() << "\n";
	LOG_INFO << "ClientSide &clientSide = ServerContains::CLIENT_SIDE_MAP.at(result.id)";
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
//		LOG_INFO << "ClientEventCode::CODE_SHOW_POKERS";
//		for (auto & client: ServerContains::CLIENT_SIDE_MAP)
//		{
//
//			std::vector<Poker> pokers = client.second->getPokers();
////			ServerTransferData dataToTrans;
////			dataToTrans.setPokers(pokers);
//			LOG_INFO << "pokers.size(): " << pokers.size();
//			LOG_INFO << PokerHelper::printPokers(pokers);
//			LOG_INFO << "CODE_SHOW_POKERS";
//			std::string dataToTransStr = SerializeHelper::SerializeToString<std::vector<Poker> >(pokers);
//			pushDataToClient(codec, conn, ClientEventCode::CODE_SHOW_POKERS, dataToTransStr);
//		}
		LOG_INFO << "ServerEventCode::CODE_GAME_STARTING";
		ServerEventListener_CODE_GAME_STARTING(codec, conn,
						ServerEventCode::CODE_GAME_STARTING, data);
		LOG_INFO << "ServerEventCode::CODE_GAME_STARTING";
	}
	else
	{
		muduo::Answer answer;
		answer.set_id(int(ClientEventCode::CODE_PVE_DIFFICULTY_NOT_SUPPORT));
		answer.add_solution("CODE_PVE_DIFFICULTY_NOT_SUPPORT");
		answer.set_questioner("san");
		answer.set_answerer("san");
		codec->send(conn, answer);
	}
}

void ServerEventListener_CODE_GAME_LANDLORD_ELECT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code, const std::string &data)
{
	LOG_INFO << "ServerEventListener_CODE_GAME_LANDLORD_ELECT";
	ClientTransferData result = SerializeHelper::parseStringToData<ClientTransferData>(data);
	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.find(result.id)->second;

	LOG_INFO << result.data;
	Room *room = ServerContains::getRoom(clientSide->getRoomId());

	LOG_INFO << "roomId: " << room->getId();

	if (result.data == "true")
	{
		LOG_INFO << "true";
		std::copy(room->getLoadlordPokers()->begin(),
				  room->getLoadlordPokers()->end(),
				  std::back_inserter(clientSide->getPokers()));

		PokerHelper::sortPoker(clientSide->getPokers());

		int currentClientId = clientSide->getId();
		room->setLandlordId(currentClientId);
		room->setLastSellClient(currentClientId);
		room->setCurrentSellClient(currentClientId);

		for (ClientSide *client: room->getClientSideList())
		{
			ServerTransferData result;
			result.setRoomId(room->getId());
			result.setRoomOwner(room->getRoomOwner());
			result.setRoomClientCount(room->getClientSideList().size());
			result.setLandlordNickname(clientSide->getNickname());
			result.setAdditionalPokers(room->getLandlordPokers());
			std::string res = SerializeHelper::SerializeToString(result);

			if (client->getRole() == ClientRole::PLAYER)
			{
				LOG_INFO << "ClientEventCode::CODE_GAME_LANDLORD_CONFIRM";
				pushDataToClient(codec, conn,
						ClientEventCode::CODE_GAME_LANDLORD_CONFIRM,
						res);
			}
			else
			{
				if(currentClientId == client->getId())
				{
					LOG_INFO << "RobotEventListener::get(codec, conn, ClientEventCode::CODE_GAME_POKER_PLAY, client, data)";
					RobotEventListener::get(codec, conn, ClientEventCode::CODE_GAME_POKER_PLAY, client, data);
				}
			}
		}
	}
}

void ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		ServerEventCode code, const std::string &data)
{
	LOG_INFO << "ServerEventListener_CODE_GAME_POKER_PLAY_REDIRECT";
	ClientTransferData resultReceive = SerializeHelper::parseStringToData<ClientTransferData>(data);
	ClientSide *clientSide = ServerContains::CLIENT_SIDE_MAP.find(resultReceive.id)->second;

	LOG_INFO << "resultReceive.id: " << resultReceive.id;
	LOG_INFO << "*clientSide";
	// FIXME
	Room *room = ServerContains::getRoom(clientSide->getRoomId());
	LOG_INFO << "clientSide->getId(): " << clientSide->getId();
	LOG_INFO << "clientSide->getPre().getId(): " << clientSide->getPre().getId();
	std::string p = clientSide->getPre().getId() == clientSide->getId() ? "up" : "down";
	LOG_INFO << "p: " << p;
	std::vector<ClientInfo> clientInfos;
	for (ClientSide *client: room->getClientSideList())
	{
		LOG_INFO << "room->getClientSideList().size(): " << room->getClientSideList().size();
		ClientInfo info(client->getId(),
						 client->getNickname(),
						 client->getType(),
						 client->getPokers().size(),
						 p);
		clientInfos.push_back(info);
	}


	ServerGamePlayData result(clientInfos,
						clientSide->getPokers(),
						std::vector<Poker>(),
						room->getLastSellClient(),
						room->getCurrentSellClient(),
						ServerContains::CLIENT_SIDE_MAP.at(room->getCurrentSellClient())->getNickname());
	LOG_INFO << "GamePlayData";
	std::string res = SerializeHelper::SerializeToString<ServerGamePlayData>(result);
	pushDataToClient(codec, conn,  ClientEventCode::CODE_GAME_POKER_PLAY_REDIRECT, res);
}

