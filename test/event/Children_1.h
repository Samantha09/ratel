/*
 * Children_1.h
 *
 *  Created on: 2021年2月24日
 *      Author: san
 */

#ifndef TEST_EVENT_CHILDREN_1_H_
#define TEST_EVENT_CHILDREN_1_H_

#include "Base.h"

class Children_1 : public Base
{
public:
	Children_1(): Base(){}

	void operator()(int order) override
	{
		printf("Children_1");
	}
};

Children_1 *ch1;

#endif /* TEST_EVENT_CHILDREN_1_H_ */
