/*
 * RobotEventListener.cc
 *
 *  Created on: 2021年2月27日
 *      Author: san
 */


#include "RobotEventListener.h"

std::unordered_map<ClientEventCode, RobotEventListener::RobotEventFunc> RobotEventListener::LISTENER_MAP =
		std::unordered_map<ClientEventCode, RobotEventListener::RobotEventFunc>();
