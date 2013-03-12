/*
 * iodemuxer.cpp
 *
 *  Created on: 2013-3-7
 *      Author: LiuYongJin
 */

#include "iodemuxer.h"
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#define _lock(plock) pthread_mutex_lock((pthread_mutex_t*)plock)
#define _unlock(plock) pthread_mutex_unlock((pthread_mutex_t*)plock)

#define LOCK(plock) plock!=NULL&&_lock(plock)
#define UNLOCK(plock) plock!=NULL&&_unlock(plock)

typedef struct _timer_info
{
	HeapItem heap_item;
	uint32_t timeout;
	bool persist;
	TimerHandler *handler;
	uint64_t expire_time;    //超时时间点(ms)
}TimerInfo;

static int _timer_info_cmp(HeapItem *item0, HeapItem *item1)
{
	TimerInfo *timerinfo0 = (TimerInfo*)item0;
	TimerInfo *timerinfo1 = (TimerInfo*)item1;
	if(timerinfo0->expire_time < timerinfo1->expire_time)
		return -1;
	else if (timerinfo0->expire_time == timerinfo1->expire_time)
		return 0;
	else
		return 1;
}

IODemuxer::IODemuxer(bool thread_safe)
	:m_timer_heap(_timer_info_cmp)
	,m_timerinfo_pool(sizeof(TimerInfo))
	,m_timer_lock(NULL)
	,m_exit(false)
{
	if(thread_safe)
	{
		m_timer_lock = malloc(sizeof(pthread_mutex_t));
		assert(m_timer_lock != NULL);
		pthread_mutex_init((pthread_mutex_t*)m_timer_lock, NULL);
	}
}

IODemuxer::~IODemuxer()
{
	if(m_timer_lock != NULL)
	{
		pthread_mutex_destroy((pthread_mutex_t*)m_timer_lock);
		free(m_timer_lock);
		m_timer_lock = NULL;
	}
}

bool IODemuxer::add_timer(TimerHandler *handler, uint32_t timeout, bool persist/*=true*/)
{
	assert(handler != NULL);

	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t now_time = tv.tv_sec*1000+tv.tv_usec/1000;

	LOCK(m_timer_lock);
	TimerInfo *timer_info = (TimerInfo*)m_timerinfo_pool.get();
	if(timer_info == NULL)
	{
		UNLOCK(m_timer_lock);
		return false;
	}

	timer_info->handler = handler;
	timer_info->timeout = timeout;
	timer_info->persist = persist;
	timer_info->expire_time = now_time + timeout;
	bool result = m_timer_heap.insert((HeapItem*)timer_info);
	if(!result)
		m_timerinfo_pool.recycle((void*)timer_info);
	UNLOCK(m_timer_lock);
	return result;
}

bool IODemuxer::run_loop()
{
	m_exit = false;
	const int WAIT_TIME = 1000;    //1000ms
	uint64_t now_time;    //当前时间(单位ms)
	int wait_time;

	while(!m_exit)
	{
		wait_time = WAIT_TIME;

		struct timeval tv;
		gettimeofday(&tv, NULL);
		now_time = tv.tv_sec*1000+tv.tv_usec/1000;

		//收集时钟超时事件并设置wait_time
		LOCK(m_timer_lock);
		TimerInfo *timer_info;
		while((timer_info=(TimerInfo*)m_timer_heap.top()) != NULL)
		{
			if(timer_info->expire_time <= now_time)    //时钟超时
			{
				m_timer_timeout_list.push_back((void*)timer_info);
				m_timer_heap.pop();
				continue;
			}
			if((wait_time=timer_info->expire_time-now_time) > WAIT_TIME)
				wait_time = WAIT_TIME;
			break;
		}
		UNLOCK(m_timer_lock);

		dispatch_events(wait_time);    //处理io事件

		//处理发生的时钟超时事件
		while(!m_timer_timeout_list.empty())
		{
			list<void*>::iterator it = m_timer_timeout_list.begin();
			TimerInfo *timer_info = (TimerInfo*)*it;
			m_timer_timeout_list.pop_front();

			if(timer_info->handler->on_timer_timeout()==HANDLE_SUCCESS && timer_info->persist)
			{
				timer_info->expire_time = now_time+timer_info->timeout;
				LOCK(m_timer_lock);
				bool temp = m_timer_heap.insert((HeapItem*)timer_info);
				UNLOCK(m_timer_lock);
				assert(temp == true);
			}
			else
			{
				LOCK(m_timer_lock);
				m_timerinfo_pool.recycle((void*)timer_info);
				UNLOCK(m_timer_lock);
			}
		}
	}

	return true;
}
