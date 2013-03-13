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
#include <errno.h>

#include <list>
using std::list;

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

IODemuxerEpoll::IODemuxerEpoll(bool et_mode/*=false*/)
	:m_eventinfo_pool(sizeof(EventInfo), 1024)
	,m_timeout_heap(_event_cmp)
	,m_et_mode(et_mode)
{
	m_epfd = epoll_create1(EPOLL_CLOEXEC);
	assert(m_epfd != -1);

	m_size = 1024;
	m_epoll_events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*m_size);
	assert(m_epoll_events != NULL);
}

IODemuxerEpoll::~IODemuxerEpoll()
{
	close(m_epfd);
	free(m_epoll_events);
}

bool IODemuxerEpoll::add_event(uint32_t fd, EventType type, EventHandler *handler, uint32_t timeout)
{
	assert(handler!=NULL && (type&ET_RDWT));
	EventInfo *event_info = NULL;

	EventInfoMap::iterator ev_it = m_eventinfo_map.find(fd);
	if(ev_it == m_eventinfo_map.end())
	{
		//从内存池中获取eventinfo对象
		event_info = (EventInfo*)m_eventinfo_pool.get();
		if(event_info == NULL)  //out of memory
		{
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
			return false;
		}

		//添加到timeout_heap中
		if(m_timeout_heap.insert((HeapItem*)event_info) != true)
		{
			m_eventinfo_map.erase(fd);
			m_eventinfo_pool.recycle((void*)event_info);
			return false;
		}

		//添加到epoll中
		struct epoll_event event;
		event.events = 0;
		event.data.ptr = (void*)event_info;
		if(type & ET_WRITE)
			event.events |= EPOLLOUT;
		if(type & ET_READ)
			event.events |= EPOLLIN;
		if(m_et_mode)
			event.events |= EPOLLET;
		if(epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &event) == -1)
		{
			if(errno != EEXIST)
			{
				m_timeout_heap.remove((HeapItem*)event_info);
				m_eventinfo_map.erase(fd);
				m_eventinfo_pool.recycle((void*)event_info);
				return false;
			}
			else if(epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &event) == -1)
			{
				return false;
			}
		}
	}
	else
	{
		//find in eventinfo_map
		event_info = (EventInfo*)ev_it->second;
		if((event_info->type&ET_RDWT) != (type&ET_RDWT))
		{
			struct epoll_event event;
			event.events = 0;
			event.data.ptr = (void*)event_info;
			if(type & ET_WRITE)
				event.events |= EPOLLOUT;
			if(type & ET_READ)
				event.events |= EPOLLIN;
			if(m_et_mode)
				event.events |= EPOLLET;
			if(epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &event) == -1)
			{
				if(errno != ENOENT)
					return false;
				else if(epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &event) == -1)
				{
					return false;
				}
			}
			event_info->type |= type;
		}
	}

	return true;
}

bool IODemuxerEpoll::delete_event(uint32_t fd)
{
	EventInfoMap::iterator ev_it = m_eventinfo_map.find(fd);
	if(ev_it != m_eventinfo_map.end())
	{
		if(epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL)==-1 && errno!=ENOENT)
			return false;
		EventInfo *event_info = (EventInfo*)ev_it->second;
		m_timeout_heap.remove((HeapItem*)event_info);
		m_eventinfo_map.erase(fd);
		m_eventinfo_pool.recycle((void*)event_info);
	}

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
		if(epoll_ctl(m_epfd, EPOLL_CTL_DEL, event_info->fd, NULL)==-1 && errno!=ENOENT)    //删除失败,不添加到超时队列(防止下面重复添加)
			continue;
		m_timeout_heap.pop();
		m_eventinfo_map.erase(event_info->fd);
		m_event_list.push_back(event_info);
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

	//处理发生的io事件
	while(!m_event_list.empty())
	{
		EventInfo *event_info = *m_event_list.begin();
		m_event_list.pop_front();
		if(event_info->occur_event & OE_TIMEOUT)    //io超时
		{
			event_info->handler->on_fd_timeout(event_info->fd);
			continue;
		}

		EventType new_type = event_info->type;
		bool error = event_info->occur_event&OE_ERROR!=0;

		if(!error && (event_info->occur_event&OE_READ))
		{
			HANDLE_RESULT handle_result = event_info->handler->on_fd_readable(event_info->fd);
			if(handle_result == HANDLE_ERROR)    //失败
				error = true;
			if(handle_result == HANDLE_FINISH)     //移除读事件
				new_type &= ~ET_READ;
		}
		if(!error && (event_info->occur_event&OE_WRITE))
		{
			HANDLE_RESULT handle_result = event_info->handler->on_fd_writeable(event_info->fd);
			if(handle_result == HANDLE_ERROR)
				error = true;
			else if(handle_result == HANDLE_FINISH)     //移除写事件
				new_type &= ~ET_WRITE;
		}

		if(error || new_type==0)
		{
			if(error)
				event_info->handler->on_fd_error(event_info->fd);
			epoll_ctl(m_epfd, EPOLL_CTL_DEL, event_info->fd, NULL);
			m_timeout_heap.remove((HeapItem*)event_info);
			m_eventinfo_map.erase(event_info->fd);
			m_eventinfo_pool.recycle((void*)event_info);
		}
		else if(new_type != event_info->type)
		{
			struct epoll_event event;
			event.events = 0;
			event.data.ptr = event_info;
			if(new_type & ET_READ)
				event.events |= EPOLLIN;
			if(new_type & ET_WRITE)
				event.events |= EPOLLOUT;
			if(m_et_mode)
				event.events |= EPOLLET;
			if(epoll_ctl(m_epfd, EPOLL_CTL_MOD, event_info->fd, &event)==-1 &&errno==ENOENT)
				if(epoll_ctl(m_epfd, EPOLL_CTL_ADD, event_info->fd, &event) == -1)
					assert(0);
			event_info->type = new_type;
		}
	}

	return;
}
