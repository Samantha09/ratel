/*
 * eventFuncs.cc
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */

#include "ClientEventListener.h"
#include "EventFuns.h"
#include "muduo/base/Mutex.h"

void pushDataToServer(ProtobufCodec *codec,
					  const muduo::net::TcpConnectionPtr &conn,
					  ServerEventCode code,
					  const MapHelper &mapHelper)
{
//	  LOG_DEBUG << "pushDataToClient " << int(code);
	  LOG_DEBUG << SERVEREVENTCODE[int(code)];
	  std::string result = SerializeHelper::SerializeToString<MapHelper>(mapHelper);
	  muduo::Query query;
	  query.set_questioner("san");
	  query.set_id(int(code));
	  query.add_question(result);
	  codec->send(conn, query);
}

// CODE_CLIENT_EXIT
void ClientEventListener_CODE_CLIENT_EXIT(ProtobufCodec *codec,
										  const muduo::net::TcpConnectionPtr &conn,
										  int clientId,
										  const MapHelper &data)
{
	LOG_DEBUG << "CODE_CLIENT_EXIT";
	exit(0);
}

void ClientEventListener_CODE_ROOM_CREATE_SUCCESS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
	  	  	  	    							  int clientId, const MapHelper &data)
{
	assert(data.get("roomId", 0) != 0);
	std::cout << "You have created a room with id " << data.get("roomId", 0)
			  << std::endl;
	std::cout << "Please wait for other players to join !" << std::endl;
}

void ClientEventListener_CODE_ROOM_JOIN_SUCCESS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
	  	    									int clientId, const MapHelper &data)
{
	LOG_DEBUG << "有人加入了房间";
	int joinClientId = data.get("joinClientId", 0);
	assert(joinClientId != 0);
	if (clientId == joinClientId)
	{
		std::cout << "You have joined room：" << data.get("roomId", 0) << ". There are currently "
				  << data.get("roomClientCount", 0) << " players in the room now." << std::endl;
		std::cout << "Please wait for other players to join. The game would start at three players!"
				  << std::endl;
	}
	else
	{
		std::cout << data.get("clientNickname", "") <<  " joined room, there are currently "
				  << data.get("roomClientCount", 0) << " in the room." << std::endl;
	}
}

void ClientEventListener_CODE_SHOW_ROOMS(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
	  	  	  	  	  	  	  	  	  	 int clientId, const MapHelper &data)
{
	std::cout << "\t" << "ID" << "\t|" << "OWNER" << "\t|" << "COUNT" << "\t|"
			  << "TYPE" << std::endl;
	std::vector<RoomInfo> roomInfos = data.get("roomInfos", std::vector<RoomInfo>());
	assert(!roomInfos.empty());
	if (!roomInfos.empty())
	{
		for (const RoomInfo &ri: roomInfos)
		{
			std::cout << "\t" << ri.roomId << "\t|" << ri.roomOwner << "\t|"
					  << ri.roomClientCount << "\t|" << (ri.roomType ? "PVE" : "PVP") << std::endl;
		}
		ClientEventListener_CODE_SHOW_OPTION_PVP(codec, conn, clientId, data);
	}
	else
	{
		std::cout << "No available room. Please create a room!" << std::endl;
		ClientEventListener_CODE_SHOW_OPTION_PVP(codec, conn, clientId, data);
	}

}

// SHOW_POKERS
void ClientEventListener_CODE_SHOW_POKERS(ProtobufCodec *codec,
										  const muduo::net::TcpConnectionPtr &conn,
										  int clientId,
										  const MapHelper &data)
{
	LOG_DEBUG << "ClientEventListener_CODE_SHOW_POKERS";
	std::string lastSellClientNickname = data.get("clientNickname", "");
	std::string lastSellClientType = (int(data.get("clientType", 0)) ? "PEASANT" : "LANDLORD");

	std::cout << lastSellClientNickname << "[" << lastSellClientType
			  << "] played: " << std::endl;

	std::vector<Poker> lastPokers = data.get("pokers", std::vector<Poker>());

	std::cout << PokerHelper::printPokers(lastPokers)
	          << std::endl;

	if (data.get("sellClinetNickname", "") != "")
	{
		std::cout << "Next player is: " << data.get("sellClinetNickname", "") << ". Please wait for him to play his combination."
				  << std::endl;
	}
}

void ClientEventListener_CODE_SHOW_OPTIONS(ProtobufCodec *codec,
										   const muduo::net::TcpConnectionPtr &conn,
										   int clientId,
										   const MapHelper &data)
{
	LOG_INFO << "clientId: " << clientId << "\n";
	printf("Options: \n");
	printf("1. PvP\n");
	printf("2. PvE\n");
	printf("3. Settings\n");
	printf("Please select an option above (enter [exit|e] to log out)\n");

	std::string line;
	printf("select: ");
	std::cin >> line;

	std::transform(line.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写
	if (line == "exit" || line == "e")
	{
		exit(0);
	}
	else
	{
		switch(atoi(line.c_str()))
		{
		case 1:
			ClientEventListener_CODE_SHOW_OPTION_PVP(codec, conn, clientId, data);
			break;
		case 2:
			ClientEventListener_CODE_SHOW_OPTION_PVE(codec, conn, clientId, data);
			break;
		case 3:
			ClientEventListener_CODE_SHOW_OPTION_SETTING(codec, conn, clientId, data);
			break;
		default:
			printf("Invalid option, please choose again：");
			ClientEventListener_CODE_SHOW_OPTIONS(codec, conn, clientId, data);
			break;
		}
	}
}

void ClientEventListener_CODE_SHOW_OPTION_PVE(ProtobufCodec *codec,
											  const muduo::net::TcpConnectionPtr &conn,
											  int clientId,
											  const MapHelper &data)
{
	printf("PvE: \n");
	printf("1. Easy Mode \n");
	printf("2. Medium Mode \n");
	printf("3. Hard Mode \n");
	printf("Please select an option above (enter [back|b] to return to options list)\n");

	std::string line;
	printf("select: ");
	std::cin >> line;

	std::transform(line.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写
	if (line == "back" || line == "b")
	{
		ClientEventListener_CODE_SHOW_OPTIONS(codec, conn, clientId, data);
	}
	else
	{
		try {
			int order = atoi(line.c_str());
		}
		catch (const std::string &line)
		{
			printf("Invalid option, please choose again：%s", line.c_str());
		}
		if (0 < atoi(line.c_str()) && atoi(line.c_str()) < 4)
		{
//			LOG_WARN << "choose is not used! in this version!, always be easy mode!!";
			pushDataToServer(codec, conn,
							 ServerEventCode::CODE_ROOM_CREATE_PVE,
							 MapHelper().put("choose", line)
							 	        .put("clientId", clientId));
		}
		else
		{
			printf("Invalid option, please choose again：");
			ClientEventListener_CODE_SHOW_OPTION_PVE(codec, conn, clientId, data);
		}
	}
}

void ClientEventListener_CODE_GAME_POKER_PLAY_PASS(ProtobufCodec *codec,
												   const muduo::net::TcpConnectionPtr &conn,
												   int clientId,
												   const MapHelper &data)
{
	LOG_DEBUG << "ClientEventListener_CODE_GAME_POKER_PLAY_PASS";
	std::cout << data.get("clientNickname", "") << " passed. It is now "
			  << data.get("nextClientNickname", "") << "'s turn"
			  << std::endl;
	int turnClientId = data.get("nextClientId", 0);

	assert(turnClientId != 0);
	assert(data.get("clientNickname", "") != "");
	assert(data.get("nextClientNickname", "") != "");

	if (clientId == turnClientId)
	{
		LOG_DEBUG << "clientId = " << clientId << "\n" << "turnClientId: " << turnClientId;
		// FIXME: MapHelper()
		pushDataToServer(codec, conn,
						 ServerEventCode::CODE_GAME_POKER_PLAY_REDIRECT,
						 MapHelper().put("clientId", clientId));
	}
}

void ClientEventListener_CODE_SHOW_OPTION_SETTING(ProtobufCodec *codec,
												  const muduo::net::TcpConnectionPtr &conn,
												  int clientId,
												  const MapHelper &data)
{
	std::vector<Poker> pokers_;
  	Poker poker(PokerType::HEART, PokerLevel::LEVEL_K);
  	Poker poker1(PokerType::DIAMOND, PokerLevel::LEVEL_K);
  	Poker poker2(PokerType::BLANK, PokerLevel::LEVEL_K);
  	Poker poker3(PokerType::CLUB, PokerLevel::LEVEL_K);
  	Poker poker4(PokerType::HEART, PokerLevel::LEVEL_SMALL_KING);
  	pokers_.push_back(poker);
  	pokers_.push_back(poker1);
  	pokers_.push_back(poker2);
  	pokers_.push_back(poker3);
  	pokers_.push_back(poker4);
}

void ClientEventListener_CODE_SHOW_OPTION_PVP(ProtobufCodec *codec,
											  const muduo::net::TcpConnectionPtr &conn,
											  int clientId,
											  const MapHelper &data)
{
	std::cout << "PVP: " << std::endl;
	std::cout << "1. Create Room" << std::endl;
	std::cout << "2. Room List" << std::endl;
	std::cout << "3. Join Room" << std::endl;
	std::cout << "Please select an option above (enter [back|b] to return to options list)" << std::endl;

	printf("pvp: ");
	fflush(stdout);

	std::string line;
	std::cin >> line;

	std::transform(line.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写

	if (line == "back" || line == "b")
	{
		ClientEventListener_CODE_SHOW_OPTIONS(codec, conn, clientId, data);
	}
	else
	{
		int choose = std::stoi(line);

		if (choose == 1)
		{
			pushDataToServer(codec, conn,
							 ServerEventCode::CODE_ROOM_CREATE,
							 MapHelper().put("clientId", clientId));
		}
		else if (choose == 2)
		{
			pushDataToServer(codec, conn,
							 ServerEventCode::CODE_GET_ROOMS,
							 MapHelper().put("clientId", clientId));
		}
		else if (choose == 3)
		{
			std::cout << "Please enter the room id you wish to join (enter [back|b] to return to options list)"
					  << std::endl;

			std::string roomIdChoosed;
			printf("roomid:");
			fflush(stdout);
			std::cin >> roomIdChoosed;

			if (roomIdChoosed == "back" || line == "b")
			{
				ClientEventListener_CODE_SHOW_OPTION_PVP(codec, conn, clientId, data);
			}
			else
			{
				int option = std::stoi(roomIdChoosed);
				if (line == "" || option < 1)
				{
					std::cout << "Invalid option, please choose again："\
							  << std::endl;
					ClientEventListener_CODE_SHOW_OPTION_PVP(codec, conn, clientId, data);
				}
				else
				{
					pushDataToServer(codec, conn,
							         ServerEventCode::CODE_ROOM_JOIN,
							         MapHelper().put("option", option)
									 	 	 	.put("clientId", clientId));
				}
			}
		}
		else
		{
			std::cout << "Invalid option, please choose again："
					  << std::endl;
			ClientEventListener_CODE_SHOW_OPTION_PVP(codec, conn, clientId, data);
		}
	}
}

void ClientEventListener_CODE_CLIENT_NICKNAME_SET(ProtobufCodec *codec,
												  const muduo::net::TcpConnectionPtr &conn,
												  int clientId,
												  const MapHelper &data)
{
	LOG_DEBUG << "测试客户端ID： " << clientId;
	if (data.get("invalidLength", 0) != 0)
	{
		std::cout << "Your nickname has invalid length: " << data.get("invalidLength", 0)
				  << std::endl;
	}
	std::cout << "Please set your nickname (upto " << 20 << " characters)"
			  << std::endl;
	std::string nick_name;
	std::cin >> nick_name;
	if (nick_name.size() > 20)
		ClientEventListener_CODE_CLIENT_NICKNAME_SET(codec, conn, clientId, data);
	else
	{
		pushDataToServer(codec, conn,
						 ServerEventCode::CODE_CLIENT_NICKNAME_SET,
						 MapHelper().put("nickName", nick_name)
						 	 	 	.put("clientId", clientId));
	}
}

void ClientEventListener_CODE_GAME_STARTING(ProtobufCodec *codec,
										    const muduo::net::TcpConnectionPtr &conn,
										    int clientId,
										    const MapHelper &data)
{
	std::cout << "Game starting! Your cards are: " << std::endl;
	assert(!data.get("pokers", std::vector<Poker>()).empty());
	std::cout << PokerHelper::printPokers(data.get("pokers", std::vector<Poker>()))
			  << std::endl;
	ClientEventListener_CODE_GAME_LANDLOAD_ELECT(codec, conn, clientId, data);
}

void ClientEventListener_CODE_GAME_LANDLOAD_ELECT(ProtobufCodec *codec,
												  const muduo::net::TcpConnectionPtr &conn,
												  int clientId,
												  const MapHelper &data)
{
	LOG_DEBUG << "ClientEventListener_CODE_GAME_LANDLOAD_ELECT";
//	ServerTransferData result = SerializeHelper::parseStringToData<ServerTransferData>(data);
	int turnClientId = data.get("nextClientId", 0);

	if (data.get("preClientNickname", "") != "")
	{
		assert(data.get("preClientNickname", "") != "");
		std::cout << data.get("preClientNickname", "") << " don't rob the landlord!";
	}

	if (turnClientId == clientId)
	{
		std::cout << "It's your turn. Do you want to rob the landlord? [Y/N] (enter [exit|e] to exit current room)"
				  << std::endl;
		std::string line;
		std::cin >> line;

		std::transform(line.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写

		if (line == "exit" || line == "e")
		{
			pushDataToServer(codec, conn,
							 ServerEventCode::CODE_CLIENT_EXIT,
							 MapHelper().put("clientId", clientId));
		}
		else if (line == "y")
		{
			LOG_INFO << line;

//			pushDataToServer(codec, conn,
//							 ServerEventCode::CODE_CLIENT_EXIT,
//							 MapHelper().put("clientId", clientId)
//							 	 	 	.put("is_Y", "true"));

			pushDataToServer(codec, conn,
							 ServerEventCode::CODE_GAME_LANDLORD_ELECT,
							 MapHelper().put("clientId", clientId)
							 	 	 	.put("is_Y", "true"));
		}
		else if (line == "n")
		{
			pushDataToServer(codec, conn,
							 ServerEventCode::CODE_GAME_LANDLORD_ELECT,
							 MapHelper().put("clientId", clientId)
										.put("is_Y", "false"));
		}
		else
		{
			std::cout << "Invalid options!" << std::endl;
			ClientEventListener_CODE_GAME_LANDLOAD_ELECT(codec, conn, clientId, data);
		}
	}
	else
	{
		assert(data.get("nextClientNickname", "") != "");
		std::cout << "It's " << data.get("nextClientNickname", "") << "'s turn. Please wait patiently for his/her confirmation !"
				  << std::endl;
	}
}

void ClientEventListener_CODE_GAME_LANDLOAD_CONFIRM(ProtobufCodec *codec,
												    const muduo::net::TcpConnectionPtr &conn,
												    int clientId,
												    const MapHelper &data)
{
	LOG_DEBUG << "我是client， 地主 is me!";
//	ServerTransferData result = SerializeHelper::parseStringToData<ServerTransferData>(data);
	std::string landlordNickname = data.get("landlordNickname", "");
	std::cout << landlordNickname << " has  become the landlord and gotten three extra cards"
			  << std::endl;
	std::vector<Poker> additionalPokers = data.get("additionalPokers", std::vector<Poker>());
	assert(!additionalPokers.empty());
	std::cout << PokerHelper::printPokers(additionalPokers)
	          << std::endl;

	// FIXME: MapHelper()
	pushDataToServer(codec, conn,
					 ServerEventCode::CODE_GAME_POKER_PLAY_REDIRECT,
					 MapHelper().put("clientId", clientId));

}

void ClientEventListener_CODE_GAME_POKER_PLAY_REDIRECT(ProtobufCodec *codec,
													   const muduo::net::TcpConnectionPtr &conn,
								  					   int clientId,
								 					   const MapHelper &data)
{
	LOG_DEBUG << "我是客户端，我在做出派前的准备";
	int sellClientId = data.get("sellClientId", 0);
	assert(sellClientId != 0);
	std::vector<ClientInfo> clientInfos = data.get("clientInfos", std::vector<ClientInfo>());

	std::string choose[2]{ "up", "down" };
	std::cout << "everyone's current situations: " << std::endl;

	for (int index = 0; index < 2; ++index)
	{
		for (ClientInfo info: clientInfos)
		{
			std::string position = info.position;
			std::string str_type = int(info.type) ? "PEASANT" : "LANDLORD";
			if (position == choose[index])
			{
				std::cout << info.clientNickname.c_str() << "\t" << info.surplus << "\t" << str_type.c_str()
						  << std::endl;
			}
		}
	}

	std::cout << " " << std::endl;

	if (sellClientId == clientId)
	{
		std::cout << "sellClientId: " << sellClientId << "\n"
				  << "clientId: " << clientId << std::endl;

		ClientEventListener_CODE_GAME_POKER_PLAY(codec, conn,
												 clientId,
												 data);
	}
	else
	{
		assert(data.get("sellClinetNickname", "") != "");
		std::cout << "It's " <<  data.get("sellClinetNickname", "") << " 's turn. Please wait for him to play his cards." << std::endl;
	}
}

bool parseLine(const std::string &line, std::vector<PokerLevel> &levels)
{
//	LOG_INFO << "parseLine: " << line;
	for (auto iter = line.begin(); iter != line.end(); ++iter)
	{
		switch((*iter))
		{
		case '3':
			levels.push_back(PokerLevel::LEVEL_3);
			break;
		case '4':
			levels.push_back(PokerLevel::LEVEL_4);
			break;
		case '5':
			levels.push_back(PokerLevel::LEVEL_5);
			break;
		case '6':
			levels.push_back(PokerLevel::LEVEL_6);
			break;
		case '7':
			levels.push_back(PokerLevel::LEVEL_7);
			break;
		case '8':
			levels.push_back(PokerLevel::LEVEL_8);
			break;
		case '9':
			levels.push_back(PokerLevel::LEVEL_9);
			break;
		case '1':
			levels.push_back(PokerLevel::LEVEL_10);
			++iter;
			break;
		case '0':
			return false;
		case 'j':
		case 'J':
			levels.push_back(PokerLevel::LEVEL_J);
			break;
		case 'q':
		case 'Q':
			levels.push_back(PokerLevel::LEVEL_Q);
			break;
		case 'k':
		case 'K':
			levels.push_back(PokerLevel::LEVEL_K);
			break;
		case 'a':
		case 'A':
			levels.push_back(PokerLevel::LEVEL_A);
			break;
		case '2':
			levels.push_back(PokerLevel::LEVEL_2);
			break;
		case 'S':
			levels.push_back(PokerLevel::LEVEL_SMALL_KING);
			break;
		case 's':
			levels.push_back(PokerLevel::LEVEL_SMALL_KING);
			break;
		case 'X':
			levels.push_back(PokerLevel::LEVEL_BIG_KING);
			break;
		case 'x':
			levels.push_back(PokerLevel::LEVEL_BIG_KING);
			break;
		default:
			return false;
		}
	}
	return true;
}

void ClientEventListener_CODE_GAME_POKER_PLAY(ProtobufCodec *codec,
											  const muduo::net::TcpConnectionPtr &conn,
											  int clientId,
											  const MapHelper &data)
{
	std::vector<Poker> pokers = data.get("pokers", std::vector<Poker>());
	std::cout << "It's your turn to play, your cards are as follows: "
			  << std::endl;
	std::cout << PokerHelper::printPokers(pokers)
	          << std::endl;

	std::cout << "Please enter the combination you came up with (enter [exit|e] to exit current room, enter [pass|p] to jump current round, enter [view|v] to show all valid combinations.)"
			  << std::endl;
	std::cout << "combination: " << std::endl;

	std::string line;
	std::cin >> line;
	std::transform(std::begin(line),std::end(line), std::begin(line), ::tolower);  // 转换为小写

	if (line == "")
	{
		printf("Invalid enter");
		LOG_INFO << "Invalid enter";
	}
	else
	{
		if ( line == "pass" || line == "p")
		{
			LOG_INFO << "pass";
			pushDataToServer(codec, conn,
							 ServerEventCode::CODE_GAME_POKER_PLAY_PASS,
							 MapHelper().put("clientId", clientId));
		}
		else if (line == "exit" || line == "e")
		{
			LOG_INFO << "exit";
			pushDataToServer(codec, conn,
					ServerEventCode::CODE_CLIENT_EXIT,
					MapHelper());
		}
		else if (line == "view" || line == "v")
		{
			LOG_INFO << "view";
			if (data.get("lastSellPokers", std::vector<Poker>()).empty() ||
				data.get("lastSellClientId", 0) == 0)
			{
				std::cout << "you are the first order!!!"
						  << std::endl;
				ClientEventListener_CODE_GAME_POKER_PLAY(codec, conn, clientId, data);
				return;
			}

			std::vector<Poker> lastSellPokers = data.get("lastSellPokers", std::vector<Poker>());
			if (lastSellPokers.empty() || clientId == data.get("lastSellClientId", 0))
			{
				std::cout << "Up to you !" << std::endl;
				ClientEventListener_CODE_GAME_POKER_PLAY(codec, conn, clientId, data);
				return;
			}
			else
			{
				std::vector<PokerSell> sells =
						PokerHelper::validSells(PokerHelper::checkPokerType(lastSellPokers), pokers);
				if (sells.empty())
				{
					std::cout << "It is a pity that, there is no winning combination..."
							  << std::endl;
					ClientEventListener_CODE_GAME_POKER_PLAY(codec, conn, clientId, data);
					return;
				}

				for(int i = 0; i < sells.size(); ++i)
				{
					// FIXME:
					std::cout << "please complete here"
							  << std::endl;
				}

				while (true)
				{
					std::cout << "You can enter index to choose anyone.(enter [back|b] to go back.)"
							  << std::endl;


					std::string lineInner;
					printf("please choose: ");
					std::cin >> lineInner;
					std::transform(std::begin(lineInner),std::end(lineInner), std::begin(lineInner), ::tolower);  // 转换为小写

					if (line == "back" || line == "b")
					{
						ClientEventListener_CODE_GAME_POKER_PLAY(codec, conn, clientId, data);
						return;
					}
					else
					{
						try
						{
							int choose = atoi(lineInner.c_str());
							if (choose < 1 || choose > sells.size())
							{
								std::cout << "The input number must be in the range of 1 to " << sells.size()
										  << ".";
							}
							else
							{
								std::vector<Poker> *choosePokers = sells.at(choose - 1).getSellPokers();
								std::vector<PokerLevel> levels;
								for (Poker p: (*choosePokers))
								{
									levels.push_back(PokerLevel(p.getLevel()));
								}
								// FIXME
								pushDataToServer(codec, conn,
												 ServerEventCode::CODE_GAME_POKER_PLAY,
												 MapHelper().put("clientId", clientId));
								break;
							}

						}
						catch (double s)
						{
							std::cout << "Please input a number."
									  << std::endl;
						}
					}
				}
			}
		}	// line == "v"
		else
		{
			std::vector<PokerLevel> options;
			bool access = parseLine(line, options);

			if (access)
			{
				pushDataToServer(codec, conn,
								 ServerEventCode::CODE_GAME_POKER_PLAY,
								 MapHelper().put("clientId", clientId)
								 	 	 	.put("options", options));
			}
			else
			{
				std::cout << "Invalid enter!" << std::endl;
				LOG_INFO << "Invalid enter";
				if (!data.get("lastPokers", std::vector<Poker>()).empty())
				{
					std::cout << "Pre played: " << std::endl;
					std::cout << PokerHelper::printPokers(data.get("lastPokers", std::vector<Poker>()))
							  << std::endl;
				}

				ClientEventListener_CODE_GAME_POKER_PLAY(codec, conn, clientId, data);
				return;
			}
		}
	}
}


void ClientEventListener_CODE_GAME_POKER_PLAY_INVALID(ProtobufCodec *codec,
													  const muduo::net::TcpConnectionPtr &conn,
													  int clientId,
													  const MapHelper &data)
{
	std::cout << "This combination is invalid." << std::endl;
	LOG_INFO << "ClientEventListener_CODE_GAME_POKER_PLAY_INVALID";

	std::vector<Poker> lastPokers = data.get("lastPokers", std::vector<Poker>());
	std::string lastSellClientNickname = data.get("lastSellClientNickname", "");
	std::string lastSellClientType = data.get("lastSellClientType", 0) ? "PEASANT" : "LANDLORD";
	if (!lastPokers.empty())
	{
		std::cout << lastSellClientNickname << "[" << lastSellClientType << "]" << "played: "
				  << std::endl;
		std::cout << PokerHelper::printPokers(lastPokers)
		          << std::endl;
	}
}

void ClientEventListener_CODE_GAME_POKER_PLAY_ORDER_ERROR(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
		  	  	  	  	  	  	  	  	  	  	  	  	  int clientId, const MapHelper &data)
{
	std::cout << "It is not your turn yet. Please wait for other players!"
			  << std::endl;
}

void ClientEventListener_CODE_GAME_OVER(ProtobufCodec *codec, const muduo::net::TcpConnectionPtr &conn,
	  	  	  	      	  	  	  	    int clientId, const MapHelper &data)
{
	std::cout << "\nPlayer " << data.get("winnerNickname", "") << " ["
			  << data.get("winnerType", "") << "]" << "won the game" << std::endl;
	std::cout << "Game over, friendship first, competition second\n" << std::endl;
}
