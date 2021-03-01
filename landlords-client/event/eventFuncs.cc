/*
 * eventFuncs.cc
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */

#include "ClientEventListener.h"
#include "EventFuns.h"
#include "muduo/base/Mutex.h"

//muduo::MutexLock mutex_1;


void pushDataToServer(const muduo::net::TcpConnectionPtr &conn,
						ProtobufCodec *codec,
						int clientId,
						ServerEventCode code,
						const std::string &data,
						std::vector<PokerLevel> levels = std::vector<PokerLevel>())
{
	if (code == ServerEventCode::CODE_GAME_POKER_PLAY_REDIRECT)
	{
		muduo::Query query;
		query.set_id(int(code));
		query.set_questioner("san");
		query.add_question(data);
		codec->send(conn, query);
	}
	else
	{
		ClientTransferData res(clientId, code, data, levels);
		std::string result = SerializeHelper::SerializeToString<ClientTransferData>(res);
		muduo::Query query;
		query.set_id(int(code));
		query.set_questioner("san");
		query.add_question(result);
	    codec->send(conn, query);
	}
}

// CODE_CLIENT_EXIT
void ClientEventListener_CODE_CLIENT_EXIT(const muduo::net::TcpConnectionPtr &conn,
										  ProtobufCodec *codec,
										  int clientId,
										  ClientEventCode code,
										  const std::string &data)
{
	exit(0);
}

// SHOW_POKERS
void ClientEventListener_CODE_SHOW_POKERS(const muduo::net::TcpConnectionPtr &conn,
										  ProtobufCodec *codec,
										  int clientId,
		                                  ClientEventCode code,
										  const std::string &data)
{
	std::cerr << "ClientEventListener_CODE_SHOW_POKERS";
	CodeShowPokersData result = SerializeHelper::parseStringToData<CodeShowPokersData >(data);
//	LOG_INFO << "\n" << PokerHelper::printPokers(result.pokers_).c_str();
//	std::vector<Poker> pokers(result.pokers_);
	std::string lastSellClientNickname = result.clientNickname;
	std::string lastSellClientType = (int(result.clientType) ? "PEASANT" : "LANDLORD");
	std::cout << lastSellClientNickname << "[" << lastSellClientType
			  << "] played: " << std::endl;
	std::cout << PokerHelper::printPokers(result.lastSellPokers)
	          << std::endl;
}

void ClientEventListener_CODE_SHOW_OPTIONS(const muduo::net::TcpConnectionPtr &conn,
										   ProtobufCodec *codec,
										   int clientId,
										   ClientEventCode code,
										   const std::string &data)
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
			ClientEventListener_CODE_SHOW_OPTION_PVP(conn, codec, clientId, code, data);
			break;
		case 2:
			ClientEventListener_CODE_SHOW_OPTION_PVE(conn, codec, clientId, code, data);
			break;
		case 3:
			ClientEventListener_CODE_SHOW_OPTION_SETTING(conn, codec, clientId, code, data);
			break;
		default:
			printf("Invalid option, please choose again：");
			ClientEventListener_CODE_SHOW_OPTIONS(conn, codec, clientId, code, data);
			break;
		}
	}
}

void ClientEventListener_CODE_SHOW_OPTION_PVE(const muduo::net::TcpConnectionPtr &conn,
											  ProtobufCodec *codec,
											  int clientId,
											  ClientEventCode code,
											  const std::string &data)
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
		ClientEventListener_CODE_SHOW_OPTIONS(conn, codec, clientId, code, data);
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
			pushDataToServer(conn, codec, clientId, ServerEventCode::CODE_ROOM_CREATE_PVE, "");
		}
		else
		{
			printf("Invalid option, please choose again：");
			ClientEventListener_CODE_SHOW_OPTION_PVE(conn, codec, clientId, code, data);
		}
	}
}

void ClientEventListener_CODE_GAME_POKER_PLAY_PASS(const muduo::net::TcpConnectionPtr &conn,
		ProtobufCodec *codec, int clientId, ClientEventCode code, const std::string &data)
{
	LOG_INFO << "pass and show pokers";
	ClientEventListener_CODE_SHOW_POKERS(conn, codec, clientId, code, data);
}

void ClientEventListener_CODE_SHOW_OPTION_SETTING(const muduo::net::TcpConnectionPtr &conn,
		ProtobufCodec *codec, int clientId, ClientEventCode code, const std::string &data)
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
//  	ServerTransferData res(pokers_);
////  	SerializeHelper::SerializeToString<ServerTransferData>(res);
//  	ClientEventListener_CODE_SHOW_POKERS(conn, code, SerializeHelper::SerializeToString<ServerTransferData>(res));
}

void ClientEventListener_CODE_SHOW_OPTION_PVP(const muduo::net::TcpConnectionPtr &conn,
		ProtobufCodec *codec, int clientId, ClientEventCode code, const std::string &data)
{

}

void ClientEventListener_CODE_CLIENT_NICKNAME_SET(const muduo::net::TcpConnectionPtr &conn,
		ProtobufCodec *codec, int clientId, ClientEventCode code, const std::string &data)
{
	if (data.size() > 20)
	{
		std::cout << "Your nickname has invalid length: " << data.size()
				  << std::endl;
	}
	std::cout << "Please set your nickname (upto " << 20 << " characters)"
			  << std::endl;
	std::string nick_name;
	std::cin >> nick_name;
	if (nick_name.size() > 20)
		ClientEventListener_CODE_CLIENT_NICKNAME_SET(conn, codec, clientId, code, nick_name);
	else
	{
		ClientTransferData result(clientId, ServerEventCode::CODE_CLIENT_NICKNAME_SET, nick_name);
		std::string res = SerializeHelper::SerializeToString<ClientTransferData>(result);
		ClientEventListener().pushToServer(conn, ServerEventCode::CODE_CLIENT_NICKNAME_SET, res);
	}
}

void ClientEventListener_CODE_GAME_STARTING(const muduo::net::TcpConnectionPtr &conn,
		ProtobufCodec *codec,  int clientId, ClientEventCode code, const std::string &data)
{
	ServerTransferData result = SerializeHelper::parseStringToData<ServerTransferData>(data);
	std::cout << "Game starting! Your cards are: " << std::endl;
	std::cout << PokerHelper::printPokers(result.pokers) << "\n" << std::endl;
	ClientEventListener_CODE_GAME_LANDLOAD_ELECT(conn, codec, clientId, code, data);
}

void ClientEventListener_CODE_GAME_LANDLOAD_ELECT(const muduo::net::TcpConnectionPtr &conn,
		ProtobufCodec *codec, int clientId, ClientEventCode code, const std::string &data)
{
	LOG_INFO << "ClientEventListener_CODE_GAME_LANDLOAD_ELECT";
	ServerTransferData result = SerializeHelper::parseStringToData<ServerTransferData>(data);
	int turnClientId = result.nextClientId;

	if (result.preClientNickname != "")
	{
		LOG_INFO << "preClientNickname != ''";
		std::cout << result.preClientNickname << " don't rob the landlord!";
	}

	if (turnClientId == clientId)
	{
		LOG_INFO << "It's your trun!";
		std::cout << "It's your turn. Do you want to rob the landlord? [Y/N] (enter [exit|e] to exit current room)"
				  << std::endl;
		std::string line;
		std::cin >> line;

		std::transform(line.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写

		if (line == "exit" || line == "e")
		{
			pushDataToServer(conn, codec, clientId, ServerEventCode::CODE_CLIENT_EXIT, "");
		}
		else if (line == "y")
		{
			pushDataToServer(conn, codec, clientId,  ServerEventCode::CODE_GAME_LANDLORD_ELECT, "true");
		}
		else if (line == "n")
		{
			pushDataToServer(conn, codec, clientId, ServerEventCode::CODE_GAME_LANDLORD_ELECT, "false");
		}
		else
		{
			std::cout << "Invalid options!" << std::endl;
			ClientEventListener_CODE_GAME_LANDLOAD_ELECT(conn, codec, clientId, code, data);
		}
	}
	else
	{
		std::cout << "It's " << result.nextClientNickname << "'s turn. Please wait patiently for his/her confirmation !"
				  << std::endl;
	}
}

void ClientEventListener_CODE_GAME_LANDLOAD_CONFIRM(const muduo::net::TcpConnectionPtr &conn,
		ProtobufCodec *codec, int clientId, ClientEventCode code, const std::string &data)
{
	LOG_DEBUG << "ClientEventListener_CODE_GAME_LANDLOAD_CONFIRM";
	ServerTransferData result = SerializeHelper::parseStringToData<ServerTransferData>(data);
	std::string landlordNickname = result.landlordNickname;

	std::cout << landlordNickname << " has  become the landlord and gotten three extra cards"
			  << std::endl;
	std::vector<Poker> additionalPokers = result.additionalPokers;
	std::cout << PokerHelper::printPokers(additionalPokers)
	          << std::endl;

	ClientTransferData res(clientId, ServerEventCode::CODE_GAME_POKER_PLAY_REDIRECT, "");
	std::string dataToSend = SerializeHelper::SerializeToString<ClientTransferData>(res);

//	CodeShowPokersData dataToSend;
//	dataToSend.clientId = clientId;
//	std::string dataToSendStr = SerializeHelper::SerializeToString<CodeShowPokersData>(dataToSend);

	pushDataToServer(conn, codec, clientId,
			         ServerEventCode::CODE_GAME_POKER_PLAY_REDIRECT,
					 dataToSend);

}

void ClientEventListener_CODE_GAME_POKER_PLAY_REDIRECT(const muduo::net::TcpConnectionPtr &conn,
		ProtobufCodec *codec,  int clientId, ClientEventCode code, const std::string &data)
{
	LOG_DEBUG << "ClientEventListener_CODE_GAME_POKER_PLAY_REDIRECT";
	ServerGamePlayData result = SerializeHelper::parseStringToData<ServerGamePlayData>(data);
	int sellClientId = result.sellClientId;
	std::vector<ClientInfo> clientInfos = result.infos;

	std::string choose[2]{ "up", "down" };

	std::cout << "everyone's current situations: " << std::endl;

	for (int index = 0; index < 2; ++index)
	{
		for (ClientInfo info: clientInfos)
		{
			std::string position = info.position;
			if (position == choose[index])
			{
				LOG_INFO << "print left pokers info: ";
				std::cout << info.position << "\t" << info.clientNickname << " "
						 << std::to_string(info.surplus) << " " << std::to_string(int(info.type))
						 << std::endl;
			}
		}

		if (sellClientId == clientId)
		{
			ClientEventListener_CODE_GAME_POKER_PLAY(conn, codec, clientId, code, data);
		}
		else
		{
			std::cout << "It's " << result.sellClinetNickname << " 's turn. Please wait for him to play his cards.";
		}
	}
}

bool parseLine(const std::string &line, std::vector<PokerLevel> &levels)
{
	LOG_INFO << "parseLine: " << line;
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
		case 'X':
			levels.push_back(PokerLevel::LEVEL_BIG_KING);
			break;
		default:
			return false;
		}
	}
	return true;
}

void ClientEventListener_CODE_GAME_POKER_PLAY(const muduo::net::TcpConnectionPtr &conn,
		ProtobufCodec *codec,  int clientId, ClientEventCode code, const std::string &data)
{

	LOG_INFO << "ClientEventListener_CODE_GAME_POKER_PLAY";
	ServerGamePlayData result = SerializeHelper::parseStringToData<ServerGamePlayData>(data);
	std::vector<ClientInfo> clientInfos = result.infos;
	std::vector<Poker> pokers = result.pokers;

	std::cout << "It's your turn to play, your cards are as follows: "
			  << std::endl;
	std::cout << PokerHelper::printPokers(pokers)
	          << std::endl;

	std::cout << "Please enter the combination you came up with (enter [exit|e] to exit current room, enter [pass|p] to jump current round, enter [view|v] to show all valid combinations.)"
			  << std::endl;
	std::cout << "combination" << std::endl;

	std::string line;
	std::transform(line.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写

	std::cin >> line;
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
			pushDataToServer(conn, codec, clientId,
					ServerEventCode::CODE_GAME_POKER_PLAY_PASS,
					data);
		}
		else if (line == "exit" || line == "e")
		{
			LOG_INFO << "exit";
			pushDataToServer(conn, codec, clientId,
					ServerEventCode::CODE_CLIENT_EXIT,
					"");
		}
		else if (line == "view" || line == "v")
		{
			LOG_INFO << "view";
			if (result.lastSellPokers.empty() || result.lastSellClientId == 0)
			{
				std::cout << "Current server version unsupport this feature, need more than v1.2.4."
						  << std::endl;
				ClientEventListener_CODE_GAME_POKER_PLAY(conn, codec, clientId, code, data);
				return;
			}

			std::vector<Poker> lastSellPokers = result.lastSellPokers;
			if (lastSellPokers.empty() || clientId == result.lastSellClientId)
			{
				std::cout << "Up to you !" << std::endl;
				ClientEventListener_CODE_GAME_POKER_PLAY(conn, codec, clientId, code, data);
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
					ClientEventListener_CODE_GAME_POKER_PLAY(conn, codec, clientId, code, data);
					return;
				}

				for(int i = 0; i < sells.size(); ++i)
				{

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

					std::transform(lineInner.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写

					if (line == "back" || line == "b")
					{
						ClientEventListener_CODE_GAME_POKER_PLAY(conn, codec, clientId, code, data);
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
								pushDataToServer(conn, codec, clientId,
										ServerEventCode::CODE_GAME_POKER_PLAY, data);
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
			std::vector<PokerLevel> res;
			bool access = parseLine(line, res);

			LOG_INFO << "res.size(): " << res.size();

			if (access)
			{
				LOG_INFO << "access: " << (access ? "true" : "false");
//				ClientTransferData dataToTransfer(clientId, ServerEventCode::CODE_GAME_POKER_PLAY, "data", res);
//				LOG_INFO << "dataToTransfer.levels.size(): " << dataToTransfer.levels.size();
//				std::string dataToTransferStr = SerializeHelper::SerializeToString<ClientTransferData>(dataToTransfer);
//				ClientTransferData temp = SerializeHelper::parseStringToData<ClientTransferData>(dataToTransferStr);
//				LOG_INFO << "temp.level.size(): " << temp.levels.size();
				pushDataToServer(conn, codec, clientId,
						ServerEventCode::CODE_GAME_POKER_PLAY, "result", res);
			}
			else
			{
				std::cout << "Invalid enter!" << std::endl;
				LOG_INFO << "Invalid enter";
				if (!result.lastSellPokers.empty())
				{
					std::cout << "Pre played: " << std::endl;
					std::cout << PokerHelper::printPokers(result.lastSellPokers)
							  << std::endl;
				}

				ClientEventListener_CODE_GAME_POKER_PLAY(conn, codec, clientId, code, data);
				return;
			}
		}
	}
}

