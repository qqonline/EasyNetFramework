/*
 * iodemuxer.cpp
 *
 *  Created on: 2013-3-7
 *      Author: Administrator
 */

#include "iodemuxer.h"
#include <sys/time.h>
#include <stdint.h>
#include <assert.h>

#define _lock(lock) pthread_mutex_lock(lock)
#define _unlock(lock) pthread_mutex_unlock(lock)
#define LOCK(lock) lock!=NULL&&_lock(lock)
#define UNLOCK(lock) lock!=NULL&&_unlock(lock)

typedef enum
{
	TIMER_HANDLER,
	EVENT_HANDLER
}HandlerType;

typedef struct _event_info
{
	uint32_t fd;
	uint32_t timeout;
	uint64_t expire_time;
	EventType event_type;
	EventHandler *handler;
	OccurEvent *occur_event;  //发生的事件
}EventInfo;

typedef struct _timer_info
{
	uint64_t expire_time;    //超时时间点(ms)
	uint32_t timeout;
	bool persist;
	TimerHandler *handler;
}TimerInfo;

static int _timer_info_cmp(void *element_a, void *element_b)
{
	TimerInfo *timer_info_a = (TimerInfo*)element_a;
	TimerInfo *timer_info_b = (TimerInfo*)element_b;
	if(timer_info_a->expire_time < timer_info_b->expire_time)
		return -1;
	else if (timer_info_a->expire_time == timer_info_b->expire_time)
		return 0;
	else
		return 1;
}

IODemuxer::IODemuxer(bool thread_safe)
	:m_timer_heap(_timer_info_cmp, NULL)
	,m_timerinfo_pool(sizeof(TimerInfo))
	,m_exit(false)
	,m_event_lock(NULL)
	,m_timer_lock(NULL)
{
	if(thread_safe)
	{
		m_event_lock = malloc(sizeof(pthread_mutex_t));
		assert(m_event_lock != NULL);
		pthread_mutex_init(m_event_lock, NULL);

		m_timer_lock = malloc(sizeof(pthread_mutex_t));
		assert(m_timer_lock != NULL);
		pthread_mutex_init(m_timer_lock, NULL);
	}
}

IODemuxer::~IODemuxer()
{
	if(m_timer_lock != NULL)
	{
		pthread_mutex_destroy(m_timer_lock);
		free(m_timer_lock);
	}
	if(m_event_lock != NULL)
	{
		pthread_mutex_destroy(m_event_lock);
		free(m_event_lock);
	}
}


bool IODemuxer::add_timer(TimerHandler *handler, uint32_t timeout, bool persist/*=true*/)
{
	assert(handler != NULL);
	LOCK(m_timer_lock);
	TimerInfo *timer_info = (TimerInfo *)m_timerinfo_pool.get();
	if(timer_info == NULL)  //out of memory
	{
		UNLOCK(m_timer_lock);
		return false;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);
	timer_info->expire_time = tv.tv_sec*1000+tv.tv_usec/1000+timeout;
	timer_info->handler = handler;
	timer_info->timeout = timeout;
	timer_info->persist = persist;
	bool result = m_timer_heap.insert((void*)timer_info);
	if(!result)
		m_timerinfo_pool.recycle((void*)timer_info);
	UNLOCK(m_timer_lock);
	return result;
}

bool IODemuxer::run_loop()
{
	m_exit = false;
	const int WAIT_TIME = 1000;  //1000ms
	uint64_t now_time;	   //当前时间(单位ms)
	int wait_time;

	EventList occur_event_list;

	while(!m_exit)
	{
		wait_time = WAIT_TIME;

		struct timeval tv;
		gettimeofday(&tv, NULL);
		now_time = tv.tv_sec*1000+tv.tv_usec/1000;

		//收集时钟超时事件并设置wait time
		LOCK(m_timer_lock);
		TimerInfo *timer_info;
		while((timer_info=(TimerInfo *)m_timer_heap.top()) != NULL)
		{
			if(timer_info->expire_time <= now_time)  //时钟超时
			{
				m_timer_timeout_list.push_back(timer_info);
				m_timer_heap.pop();
				continue;
			}
			if((wait_time=timer_info->expire_time-now_time) > WAIT_TIME)
				wait_time = WAIT_TIME;
			break;
		}
		UNLOCK(m_timer_lock);

		//检查io超时
		LOCK(m_event_lock);
		EventInfoMap::iterator event_it;
		for(event_it=m_eventinfo_map.begin(); event_it!=m_eventinfo_map.end();/*none*/)
		{
			EventInfo *event_info = (EventInfo*)event_it->second;
			event_info->occur_event = NULL;
			if(event_info->expire_time <= now_time)  //io超时
			{
				m_event_timeout_list.push_back(event_info);
				m_eventinfo_map.erase(event_it++);
				remove_event(event_info->fd, RDWT);
				continue;
			}
			else
				++event_it;
		}
		UNLOCK(m_event_lock);

		//等待/处理io事件发生
		occur_event_list.clear();
		bool wait_result = wait_event(&occur_event_list, wait_time);
		while(!occur_event_list.empty())
		{
			EventList::iterator it = occur_event_list.begin();
			OccurEvent &oe = *it;

			LOCK(m_event_lock);
			EventInfoMap::iterator ei_it = m_eventinfo_map.find(oe.fd);
			assert(ei_it != m_eventinfo_map.end());
			UNLOCK(m_event_lock);

			EventInfo *event_info = (EventInfo*)ei_it->second;
			bool error = (oe.event_flags&EVENT_FLAG_ERROR)!=0;
			if(!error && (oe.event_flags&EVENT_FLAG_READ))
				error = (event_info->handler->on_fd_readable(oe.fd)!=HANDLE_SUCCESS);
			if(!error && (oe.event_flags & EVENT_FLAG_WRITE))
				error = (event_info->handler->on_fd_writeable(oe.fd) != HANDLE_SUCCESS);

			if(error)
			{
				LOCK(m_event_lock);
				remove_event(oe.fd, RDWT);
				event_info->handler->on_fd_error(oe.fd, oe.errno);
				m_eventinfo_map.erase(ei_it);
				m_eventinfo_pool.recycle((void*)event_info);
				UNLOCK(m_event_lock);
			}
		}

		//处理io超时事件
		while(!m_event_timeout_list.empty())
		{
			list<void*>::iterator it = m_event_timeout_list.begin();
			EventInfo *event_info = (EventInfo *)*it;
			event_info->handler->on_fd_timeout(event_info->fd);
			LOCK(m_timer_lock);
			m_eventinfo_pool.recycle((void*)timer_info);
			UNLOCK(m_timer_lock);
		}
		//处理发生的时钟超时事件
		while(!m_timer_timeout_list.empty())
		{
			list<void*>::iterator it = m_timer_timeout_list.begin();
			TimerInfo *timer_info = (TimerInfo *)*it;
			m_timer_timeout_list.pop_front();

			if(timer_info->handler->on_timer_timeout()==HANDLE_SUCCESS && timer_info->persist)
			{
				timer_info->expire_time = now_time+timer_info->timeout;
				LOCK(m_timer_lock);
				bool temp = m_timer_heap.insert((void*)timer_info);
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
