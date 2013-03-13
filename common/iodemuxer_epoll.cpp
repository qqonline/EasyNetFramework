/*
 * iodemuxer_epoll.cpp
 *
 *  Created on: 2013-3-11
 *      Author: LiuYongJin
 */

#include "iodemuxer_epoll.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include <list>
using std::list;

#define _lock(plock) pthread_mutex_lock((pthread_mutex_t*)plock)
#define _unlock(plock) pthread_mutex_unlock((pthread_mutex_t*)plock)

#define LOCK(plock) plock!=NULL&&_lock(plock)
#define UNLOCK(plock) plock!=NULL&&_unlock(plock)

typedef uint32_t OccureEvent;
#define OE_TIMEOUT       1
#define OE_ERROR         2
#define OE_READ          4
#define OE_WRITE         8

typedef struct _event_info_
{
	HeapItem heap_item;
	uint32_t fd;
	EventType type;
	uint32_t timeout;  //ms
	EventHandler *handler;
	OccureEvent occur_event;
	uint64_t expire_time;
}EventInfo;

static int _event_cmp(HeapItem *item0, HeapItem *item1)
{
	EventInfo *eventinfo0 = (EventInfo*)item0;
	EventInfo *eventinfo1 = (EventInfo*)item1;
	if(eventinfo0->expire_time < eventinfo1->expire_time)
		return -1;
	else if (eventinfo0->expire_time == eventinfo1->expire_time)
		return 0;
	else
		return 1;
}

IODemuxerEpoll::IODemuxerEpoll(bool thread_safe/*=true*/, bool et_mode/*=false*/)
	:IODemuxer(thread_safe)
	,m_eventinfo_pool(sizeof(EventInfo), 1024)
	,m_timeout_heap(_event_cmp)
	,m_et_mode(et_mode)
{
	m_epfd = epoll_create1(EPOLL_CLOEXEC);
	assert(m_epfd != -1);

	m_size = 1024;
	m_epoll_events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*m_size);
	assert(m_epoll_events != NULL);

	if(thread_safe)
	{
		m_event_lock = malloc(sizeof(pthread_mutex_t));
		assert(m_event_lock != NULL);
		pthread_mutex_init((pthread_mutex_t*)m_event_lock, NULL);
	}
}

IODemuxerEpoll::~IODemuxerEpoll()
{
	close(m_epfd);
	if(m_event_lock != NULL)
	{
		pthread_mutex_destroy((pthread_mutex_t*)m_event_lock);
		free(m_event_lock);
		m_event_lock = NULL;
	}
}

bool IODemuxerEpoll::add_event(uint32_t fd, EventType type, EventHandler *handler, uint32_t timeout)
{
	assert(handler != NULL);
	assert(type & RDWT);

	EventInfo *event_info = NULL;

	LOCK(m_event_lock);
	EventInfoMap::iterator ev_it = m_eventinfo_map.find(fd);
	if(ev_it == m_eventinfo_map.end())
	{
		//从内存池中获取eventinfo对象
		event_info = (EventInfo*)m_eventinfo_pool.get();
		if(event_info == NULL)  //out of memory
		{
			UNLOCK(m_event_lock);
			return false;
		}

		struct timeval tv;
		gettimeofday(&tv, NULL);
		uint64_t now_time = tv.tv_sec*1000+tv.tv_usec/1000;

		event_info->fd = fd;
		event_info->type = type;
		event_info->timeout = timeout;
		event_info->handler = handler;
		event_info->expire_time = now_time+timeout;
		event_info->occur_event = 0;

		//添加到eventinfo_map
		std::pair<EventInfoMap::iterator, bool> result;
		result = m_eventinfo_map.insert(std::make_pair(fd, (void*)event_info));
		if(result.second != true)
		{
			m_eventinfo_pool.recycle((void*)event_info);
			UNLOCK(m_event_lock);
			return false;
		}

		//添加到timeout_heap中
		bool insert_succ = m_timeout_heap.insert((HeapItem*)event_info);
		if(!insert_succ)
		{
			m_eventinfo_map.erase(fd);
			m_eventinfo_pool.recycle((void*)event_info);
			UNLOCK(m_event_lock);
			return false;
		}

		//添加到epoll中
		struct epoll_event event;
		event.events = 0;
		event.data.ptr = (void*)event_info;
		if(type & WRITE)
			event.events |= EPOLLOUT;
		if(type & READ)
			event.events |= EPOLLIN;
		if(m_et_mode)
			event.events |= EPOLLET;
		if(epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &event) == -1)
		{
			m_timeout_heap.remove((HeapItem*)event_info);
			m_eventinfo_map.erase(fd);
			m_eventinfo_pool.recycle((void*)event_info);
			UNLOCK(m_event_lock);
			return false;
		}
	}
	else
	{
		//find in eventinfo_map
		event_info = (EventInfo*)ev_it->second;
		if((event_info->type&RDWT) != (type&RDWT))
		{
			struct epoll_event event;
			event.events = 0;
			event.data.ptr = (void*)event_info;
			if(type & WRITE)
				event.events |= EPOLLOUT;
			if(type & READ)
				event.events |= EPOLLIN;
			if(m_et_mode)
				event.events |= EPOLLET;
			if(epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &event) == -1)
			{
				UNLOCK(m_event_lock);
				return false;
			}
		}
		event_info->type |= type;
		event_info->occur_event = 0;
	}

	UNLOCK(m_event_lock);
	return true;
}

bool IODemuxerEpoll::delete_event(uint32_t fd)
{
	LOCK(m_event_lock);
	EventInfoMap::iterator ev_it = m_eventinfo_map.find(fd);
	if(ev_it != m_eventinfo_map.end())
	{
		EventInfo *event_info = (EventInfo*)ev_it->second;
		m_timeout_heap.remove((HeapItem*)event_info);
		m_eventinfo_map.erase(fd);
		m_eventinfo_pool.recycle((void*)event_info);
		epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
	}
	UNLOCK(m_event_lock);
	return true;
}

void IODemuxerEpoll::dispatch_events(uint64_t now_ms, uint32_t wait_ms)
{
	if(now_ms == 0)
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		now_ms = tv.tv_sec*1000+tv.tv_usec/1000;
	}

	EventInfo *event_info;
	list<EventInfo*> m_event_list;

	//检查超时的fd
	LOCK(m_event_lock);
	if(m_size < m_eventinfo_map.size())
	{
		m_size = m_eventinfo_map.size()*2;
		m_epoll_events = (struct epoll_event*)realloc((void*)m_epoll_events, sizeof(struct epoll_event)*m_size);
		assert(m_epoll_events != NULL);
	}

	while((event_info=(EventInfo*)m_timeout_heap.top()) != NULL)
	{
		if(event_info->expire_time > now_ms)
			break;
		m_timeout_heap.pop();
		m_eventinfo_map.erase(event_info->fd);
		epoll_ctl(m_epfd, EPOLL_CTL_DEL, event_info->fd, NULL);    //失败?
		m_event_list.push_front(event_info);
		event_info->occur_event = OE_TIMEOUT;
	}

	int count = epoll_wait(m_epfd, m_epoll_events, m_size, wait_ms);
	while(--count >= 0)
	{
		event_info = (EventInfo*)m_epoll_events[count].data.ptr;
		event_info->occur_event = 0;

		if(m_epoll_events[count].events & EPOLLERR)
			event_info->occur_event |= OE_ERROR;
		if(m_epoll_events[count].events & EPOLLIN)
			event_info->occur_event |= OE_READ;
		if(m_epoll_events[count].events & EPOLLOUT)
			event_info->occur_event |= OE_WRITE;

		m_event_list.push_front(event_info);
	}

	UNLOCK(m_event_lock);

	//处理发生的io事件
	return;
}
