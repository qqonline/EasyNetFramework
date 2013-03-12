/*
 * iodemuxer_epoll.cpp
 *
 *  Created on: 2013-3-11
 *      Author: LiuYongJin
 */

#include "iodemuxer_epoll.h"
#include <assert.h>

#define _lock(plock) pthread_mutex_lock(plock)
#define _unlock(plock) pthread_mutex_unlock(plock)

#define LOCK(plock) plock!=NULL&&_lock(plock)
#define UNLOCK(plock) plock!=NULL&&_unlock(plock)

typedef struct _event_info_
{
	uint32_t fd;
	EventType type;
	uint32_t timeout;  //ms
	EventHandler *handler;
	uint64_t expire_time;
}EventInfo;


IODemuxerEpoll::IODemuxerEpoll(bool thread_safe/*=true*/, bool ET_MODE/*=false*/)
	:IODemuxer(thread_safe)
	,m_eventinfo_pool(sizeof(EventInfo), 1024)
	,m_et_mode(ET_MODE)
{
	if(thread_safe)
	{
		m_event_lock = malloc(sizeof(pthread_mutex_t));
		assert(m_event_lock != NULL);
		pthread_mutex_init(m_event_lock, NULL);
	}
}

IODemuxerEpoll::~IODemuxerEpoll()
{
	if(m_event_lock != NULL)
	{
		pthread_mutex_destroy(m_event_lock);
		free(m_event_lock);
		m_event_lock = NULL;
	}
}

bool IODemuxerEpoll::add_event(EventHandler *handler, uint32_t fd, EventType type, uint32_t timeout)
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
		event_info->expire_time = now_time + timeout;

		//添加到eventinfo_map
		std::pair<EventInfoMap::iterator, bool> result;
		result = m_eventinfo_map.insert(std::make_pair(fd, (void*)event_info));
		if(result.second != true)
		{
			m_eventinfo_pool.recycle((void*)event_info);
			UNLOCK(m_event_lock);
			return false;
		}
		//TODO: 添加到超时监控队列中
		//
		//

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
			//TODO:从超时队列删除
			//
			//

			m_eventinfo_map.erase(fd);
			m_eventinfo_pool.recycle((void*)event_info);
			UNLOCK(m_event_lock);
			return false;
		}

		UNLOCK(m_event_lock);
		return true;
	}

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
	event_info->type |= type;    //maybe different(PERSIST flag)
	UNLOCK(m_event_lock);
	return true;
}

bool IODemuxerEpoll::delete_event(uint32_t fd, EventType type)
{
	if((type&RDWT) == 0)
		return true;
	LOCK(m_event_lock);
	EventInfoMap::iterator ev_it = m_eventinfo_map.find(fd);
	if(ev_it == m_eventinfo_map.end())
	{
		UNLOCK(m_event_lock);
		return true;
	}

	//find in eventinfo_map
	EventInfo *event_info = (EventInfo*)ev_it->second;
	int new_type = ~type&event_info->type;

	struct epoll_event event;
	event.events = 0;
	event.data.ptr = (void*)event_info;
	if((new_type&RDWT) == READ)
		event.events |= EPOLLIN;
	if((new_type&WRITE) == WRITE)
		event.events |= EPOLLOUT;
	if(event.events == 0)   //remove event from epoll
	{
		//TODO:从超时队列删除掉
		//
		//

		m_eventinfo_map.erase(fd);
		m_eventinfo_pool.recycle((void*)event_info);
		if(epoll_ctl(m_epfd, EPOLL_CTL_DEL, -1, NULL) == -1)
		{
			UNLOCK(m_event_lock);
			return false;
		}
	}
	else
	{
		if(m_et_mode)
			event.events |= EPOLLET;
		if(epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &event) == -1)
		{
			UNLOCK(m_event_lock);
			return false;
		}
	}

	UNLOCK(m_event_lock);
	return true;
}

void IODemuxerEpoll::dispatch_events(uint32_t wait_ms)
{

	return;
}
