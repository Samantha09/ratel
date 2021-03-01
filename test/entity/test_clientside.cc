/*
 * test_clientside.cc
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#include "ClientSide.h"
#include "helper/SerializeHelper.h"

#include <string>

int main()
{
	ClientSide cs(1, ClientStatus::PLAYING);
	std::string res = SerializeHelper::SerializeToString<ClientSide>(cs);
	ClientSide side = SerializeHelper::parseStringToData<ClientSide>(res);
	std::cout << side.nickname_;
}
