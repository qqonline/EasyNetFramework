/*
 * EventServer.cpp
 *
 *  Created on: Apr 17, 2013
 *      Author: LiuYongJin
 */

#include "common/EventServer.h"

namespace easynet
{

IMPL_LOGGER(IODemuxer, logger);

#define WAIT_TIME    200    //io事件的等待时间取该值和最小超时时间的较小值

typedef struct _timer_info
{
	HeapItem heap_item;
	uint64_t expire_time;    //超时时间点
	EventInfo event_info;
}TimerInfo;

#define GetCurTime(now) do{              \
	struct timeval tv;                   \
	gettimeofday(&tv, NULL);              \
	now = tv.tv_sec*1000+tv.tv_usec/1000; \
}while(0)

#define SetEventInfo(event_info, f, t, h, to) do{ \
	event_info->fd      = f;                       \
	event_info->type    = t;                       \
	event_info->handler = h;                       \
	event_info->timeout = to;                      \
}while(0)

#define SetTimerInfo(timer_info, f, t, h, to) do{ \
	timer_info->heap_item.index    = -1;           \
	int64_t now;                                   \
	GetCurTime(now);                               \
	timer_info->expire_time        = now+to;       \
	timer_info->event_info.fd      = f;            \
	timer_info->event_info.type    = t;            \
	timer_info->event_info.handler = h;            \
	timer_info->event_info.timeout = to;           \
}while(0)



static int _timer_cmp(HeapItem *item0, HeapItem *item1)
{
	TimerInfo *t0 = (TimerInfo*)item0;
	TimerInfo *t1 = (TimerInfo*)item1;
	if(t0->expire_time < t1->expire_time)
		return -1;
	else if (t0->expire_time == t1->expire_time)
		return 0;
	else
		return 1;
}

EventServer::EventServer(uint32_t max_events)
	:m_TimerHeap(_timer_cmp)
	,m_ObjectPool(sizeof(TimerInfo), max_events)
	,m_CanStop(false)
	,m_MaxEvents(max_events)
{
}

bool EventServer::AddTimer(EventHandler *handler, uint32_t timeout, bool persist/*=true*/)
{
	assert(handler != NULL);
	TimerInfo *timer_info = new TimerInfo;
	if(timer_info == NULL)
	{
		LOG_ERROR(logger, "out of memory");
		return false;
	}
	EventType type = persist?ET_PERSIST:ET_EMPTY;
	SetTimerInfo(timer_info, -1, type, handler, timeout);
	if(m_TimerHeap.Insert((HeapItem*)timer_info))
		return true;
	//failed
	LOG_ERROR(logger, "add timer failed.");
	delete timer_info;
	return false;
}

bool EventServer::AddEvent(int32_t fd, EventType type, EventHandler *handler, int32_t timeout)
{
	if(fd<=0 || ET_IS_EMPTY(type) || handler==NULL)
	{
		LOG_WARN(logger, "parameters invalid. fd="<<fd<<" type="<<type);
		return false;
	}

	EventMap::iterator it = m_EventMap.find(fd);
	if(it == m_EventMap.end())
	{
		TimerInfo *timer_info = (TimerInfo *)m_ObjectPool.Get();
		if(timer_info == NULL)
		{
			LOG_ERROR(logger, "out of memory. fd="<<fd<<" type="<<type);
			return false;
		}
		//保存到event map
		std::pair<EventMap::iterator, bool> result = m_EventMap.insert(std::make_pair(fd, (void*)timer_info));
		if(result.second == false)
		{
			LOG_ERROR(logger, "insert event_map failed. fd="<<fd<<" type="<<type);
			m_ObjectPool.Recycle((void*)timer_info);
			return false;
		}

		EventInfo *event_info = &(timer_info->event_info);
		//添加时钟
		if(timeout >= 0)
		{
			SetTimerInfo(timer_info, fd, type, handler, timeout);
			if(!m_TimerHeap.Insert((HeapItem*)timer_info))
			{
				LOG_ERROR(logger, "add timer failed. fd="<<fd<<" type="<<type);
				m_EventMap.erase(fd);
				m_ObjectPool.Recycle((void*)timer_info);
				return false;
			}
		}
		else
		{
			SetEventInfo(event_info, fd, type, handler, timeout);
		}

		//添加事件
		if(!AddEvent(fd, type, event_info))
		{
			LOG_ERROR(logger, "add event failed. fd="<<fd<<" type="<<type);
			m_EventMap.erase(fd);
			if(timeout >= 0)
				m_TimerHeap.Remove((HeapItem*)timer_info);
			m_ObjectPool.Recycle((void*)timer_info);
			return false;
		}

		return true;
	}

	//find in event map
	EventInfo *event_info = (EventInfo *)it->second;
	EventType old_type = event_info->type&ET_RDWT;
	EventType new_type = type&ET_RDWT;
	if(new_type == old_type)
	{
		LOG_DEBUG(logger, "new event_type is equal to the old. nothing to do");
		event_info->type |= type;    //maybe EV_PERSIST
		return true;
	}

	type |= event_info->type;
	if(!ModifyEvent(fd, type))
	{
		LOG_ERROR(logger, "modify event failed and delete all events. fd="<<fd);
		DelEvent(fd);
		m_EventMap.erase(fd);
		if(event_info->timeout >= 0)
			m_TimerHeap.Remove((HeapItem*)event_info);
		m_EventInfoPool.Recycle((void*)event_info);
		return false;
	}

	event_info->type = type;
	return true;
}

bool EventServer::DelEvent(int32_t fd, EventType type)
{
	if(fd<=0 || ET_IS_EMPTY(type))
	{
		LOG_INFO(logger, "parameters invalid. fd="<<fd<<" type="<<type);
		return true;
	}

	EventMap::iterator it = m_EventMap.find(fd);
	if(it == m_EventMap.end())
	{
		LOG_INFO(logger, "no found in event_map. fd="<<fd<<" type="<<type);
		return true;
	}

	EventInfo *event_info = (EventInfo *)it->second;
	EventType old_type = event_info->type&ET_RDWT;
	EventType new_type = type&ET_RDWT;
	new_type = old_type & ~new_type;
	if(new_type == 0)
	{
		if(!DelEvent(fd))
		{
			LOG_ERROR(logger, "delete event failed. fd="<<fd);
			return false;
		}
		m_EventMap.erase(fd);
		if(event_info->timeout >= 0)
			m_TimerHeap.Remove((HeapItem*)event_info);
		m_EventInfoPool.Recycle((void*)event_info);
		return true;
	}

	if(!ModifyEvent(fd, new_type))
	{
		LOG_ERROR(logger, "modify event failed and delete all events. fd="<<fd);
		return false;
	}

	event_info->type &= ~type;
	return true;
}

}//namespace

