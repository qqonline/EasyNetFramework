/*
 * CondQueue_eaxmple.cpp
 *
 *  Created on: 2013-6-29
 *      Author: tim
 */

#include "CondQueue.h"
using namespace easynet;

#include <stdio.h>

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
	return 0;
}
