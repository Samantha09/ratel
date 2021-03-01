/*
 * ClientEventCode.CODE_SHOW_OPTIONS.h
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */

#ifndef LANDLORDS_CLIENT_EVENT_CLIENTEVENTCODE_CODE_SHOW_OPTIONS_H_
#define LANDLORDS_CLIENT_EVENT_CLIENTEVENTCODE_CODE_SHOW_OPTIONS_H_

#include <stdio.h>
#include <string>
#include <algorithm>

void show_option()
{
	printf("Options: ");
	printf("1. PvP");
	printf("2. PvE");
	printf("3. Settings");
	printf("Please select an option above (enter [exit|e] to log out)");

	std::string line;
	scanf("select: %s", &line);

	std::transform(line.begin(),line.end(), line.begin(), ::tolower);  // 转换为小写
	if (line == "exit" || line == "e")
	{
		exit(0);
	}
	else
	{
		switch(line)
		{
		case "1":

		}
	}
}


#endif /* LANDLORDS_CLIENT_EVENT_CLIENTEVENTCODE_CODE_SHOW_OPTIONS_H_ */
