/*
 * CondQueue_eaxmple.cpp
 *
 *  Created on: 2013-6-29
 *      Author: tim
 */

#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include "CondQueue.h"
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
	ThreadA(int index, CondQueue<ThreadA*> *queue):m_Index(index),m_Queue(queue)
	{
		sem_init(&m_sem, 0, 0);
	}
	void Set()
	{
		sem_post(&m_sem);
	}
	int GetIndex(){return m_Index;}
protected:
	void DoRun()
	{
		while(true)
		{
			sem_wait(&m_sem);
			printf("#Thread=%d run\n", m_Index);
			sleep(1);
			m_Queue->Push(this);
			printf("#Thread=%d push\n", m_Index);
		}
	}
private:
	int m_Index;
	CondQueue<ThreadA*> *m_Queue;
	sem_t m_sem;
};

void test_thread()
{
	CondQueue<ThreadA*> condqueue(4);

	ThreadA a(1, &condqueue);
	ThreadA b(2, &condqueue);
	ThreadA c(3, &condqueue);
	ThreadA d(4, &condqueue);
	condqueue.Push(&a);
	condqueue.Push(&b);
	condqueue.Push(&c);
	condqueue.Push(&d);

	a.StartThread();
	b.StartThread();
	c.StartThread();
	d.StartThread();

	while(true)
	{
		ThreadA *thread;
		condqueue.Pop(thread);
		printf("***pop Thread=%d\n", thread->GetIndex());
		thread->Set();
	}
}

int main()
{
	test_thread();
	return 0;
}
