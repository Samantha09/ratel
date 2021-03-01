/*
 * RobotDecisionMakers.cc
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */


#include "RobotDecisionMakers.h"

std::unordered_map<int, RobotDecisionMakers::PlayPokersFunc> RobotDecisionMakers::PlayPokersFuncMap =
		std::unordered_map<int, RobotDecisionMakers::PlayPokersFunc>();
std::unordered_map<int, RobotDecisionMakers::ChooseLandlordFunc> RobotDecisionMakers::ChooseLandlordFuncMap =
		std::unordered_map<int, RobotDecisionMakers::ChooseLandlordFunc>();

