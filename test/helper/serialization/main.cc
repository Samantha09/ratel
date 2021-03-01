/*
 * main.cc
 *
 *  Created on: 2021年2月26日
 *      Author: san
 */

#include "SerializeHelper.h"

int main()
{
	ServerTransferData result(0);
	result.setRoomId(0);
	result.setRoomOwner("");
	result.setRoomClientCount(4);
	result.setnextClientNickname("");
	result.setNextClientId(3);
	result.setPokers(std::vector<Poker>());
	result.setClientOrderList(std::vector<ClientSide>());
	std::string data = SerializeHelper::SerializeToString<ServerTransferData>(result);
	ServerTransferData res = SerializeHelper::parseStringToData<ServerTransferData>(data);
	std::cout << res.nextClientNickname;
	res.roomClientList.push_back(ClientSide());
	std::cout << res.roomClientList[0].id_ << std::endl;
}


