/*
 * CondQueue_eaxmple.cpp
 *
 *  Created on: 2013-6-29
 *      Author: tim
 */

#include <stdio.h>

#include "CondQueue.h"
#include "TCondQueue.h"
using namespace easynet;


class B
{
public:
	B(){}
	~B(){printf("~B()\n");}
};

int main()
{
	CondQueue cond_queue(10);
	void *elem = NULL;
	bool ret = cond_queue.Pop(elem, 3000);
	if(elem == false)
	{
		printf("no element.\n");
	}

	printf("push element...\n");
	cond_queue.Push((void*)10, -1);
	ret = cond_queue.Pop(elem, 3000);
	if(ret != false)
	{
		printf("element=%d\n",(int)elem);
	}

	TCondQueue<int> tcond_queue(100);
	int i;
	ret = tcond_queue.Pop(i, 1000);
	if(ret == false)
	{
		printf("TCondQueue: no element.\n");
	}

	for(i=10;i<50;++i)
		tcond_queue.Push(i, -1);
	int count = 0;
	while(tcond_queue.Pop(i, 1000))
	{
		printf("TCondQueue: element=%d  ", i);
		++count;
		if(count % 5 == 0)
			printf("\n");
	}

	TCondQueue<B> tcond_queue1(100);
	B b;
	for(i=10;i<20;++i)
		tcond_queue1.Push(b, -1);
	tcond_queue1.Pop(b, -1);
	tcond_queue1.Pop(b, -1);
	printf(".....\n");

	return 0;
}
