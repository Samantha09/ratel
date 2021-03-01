

#ifndef TEST_EVENT_BASE_H_
#define TEST_EVENT_BASE_H_

#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <functional>

class Base {
public:

	Base();
	virtual ~Base(){}

	virtual void operator()(int order)
	{
		printf("Base");
		(*(solveFuncMap[order]))(order);
	}

private:
	static void init();

public:
	static std::unordered_map<int, Base*> solveFuncMap;
	static int id;
};


#endif
