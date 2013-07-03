/*
 * CondQueue_eaxmple.cpp
 *
 *  Created on: 2013-6-29
 *      Author: tim
 */

#include <stdio.h>
#include <semaphore.h>
#include "CondQueue.h"
#include "TCondQueue.h"
#include "Thread.h"
using namespace easynet;


class B
{
public:
	B(){}
	~B(){printf("~B()\n");}
};



//////////////////////
class ThreadA:public Thread
{
public:
	ThreadA(CondQueue *queue):m_Queue(queue)
	{
		sem_init(&m_sem, 0, 0);
	}
	void Set()
	{
		sem_post(&m_sem);
	}
protected:
	void DoRun()
	{
		while(true)
		{
			m_Queue->Push((void*)this, -1);
		}
	}
private:
	CondQueue *m_Queue;
	sem_t m_sem;
};

void test_thread()
{
	CondQueue condqueue(4);

	ThreadA a(&condqueue),b(&condqueue),c(&condqueue),d(&condqueue);
	condqueue.Push((void*)&a, -1);
	condqueue.Push((void*)&b, -1);
	condqueue.Push((void*)&c, -1);
	condqueue.Push((void*)&d, -1);

	a.Start();
	b.Start();
	c.Start();
	d.Start();

	while(true)
	{
		ThreadA *thread;
		void *temp;
		condqueue.Pop(temp);
		((ThreadA *)temp)->Set();
	}
}

int main()
{
//#define A
#ifdef A
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
#else
	test_thread();
#endif
	return 0;
}
