/*
 * Base.cc
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */


#include "Base.h"
#include "Children_1.h"

int Base::id = 0;
std::unordered_map<int, Base*> Base::solveFuncMap =
		std::unordered_map<int, Base*>();

Base::Base()
{
	init();
}


void Base::init()
{
	solveFuncMap.insert(std::make_pair(1, ch1));
}
