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

typedef struct _event_info
{
	HeapItem heap_item;
	int32_t fd;
	EventType type;
	EventHandler *handler;
	uint32_t timeout;        //超时时间(ms)
	uint64_t expire_time;    //超时时间点(ms)
}EventInfo;

static int _event_info_cmp(HeapItem *item0, HeapItem *item1)
{
	EventInfo *e_a = (EventInfo*)item0;
	EventInfo *e_b = (EventInfo*)item1;
	if(e_a->expire_time < e_a->expire_time)
		return -1;
	else if (e_a->expire_time == e_a->expire_time)
		return 0;
	else
		return 1;
}

EventServer::EventServer(uint32_t max_num)
	:m_TimerHeap(_event_info_cmp)
	,m_EventInfoPool(sizeof(EventInfo), max_num)
	,m_CanStop(false)
{
}

void* EventServer::_AddTimer(int32_t fd, EventType type, EventHandler *handler, uint32_t timeout_ms)
{
	EventInfo *event_info;
	if((event_info=(EventInfo*)m_EventInfoPool.Get()) == NULL)
	{
		LOG4CPLUS_ERROR(logger, "event_info_pool out of memory. fd="<<fd<<" type="<<type);
		return NULL;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t now_time = tv.tv_sec*1000+tv.tv_usec/1000;

	event_info->heap_item.index = -1;
	event_info->fd = fd;
	event_info->type = type;
	event_info->handler = handler;
	event_info->timeout = timeout_ms;
	event_info->expire_time = now_time+timeout_ms;
	if(!m_TimerHeap.Insert((HeapItem*)event_info))
	{
		m_EventInfoPool.Recycle((void*)event_info);
		LOG4CPLUS_ERROR(logger, "insert event_info into timer_heap failed. fd="<<fd<<" type="<<type);
		event_info = NULL;
	}
	LOG4CPLUS_DEBUG(logger, "insert timer success. fd="<<fd<<" type="<<type<<" timeout="<<event_info->timeout<<" expire="<<event_info->expire_time);

	return (void*)event_info;
}

bool EventServer::AddTimer(EventHandler *handler, uint32_t timeout_ms, bool persist/*=true*/)
{
	assert(handler != NULL);
	EventType type = persist?EV_PERSIST:EV_EMPTY;
	return _AddTimer(-1, type, handler, timeout_ms) != NULL;
}

bool EventServer::AddEvent(int32_t fd, EventType type, EventHandler *handler, uint32_t timeout_ms)
{
	if(fd<=0 || EV_IS_EMPTY(type) || handler==NULL)
		return false;
	EventInfo *event_info = (EventInfo*)_AddTimer(fd, type, handler, timeout_ms);
	if(event_info == NULL)
		return false;
	if(!AddEvent(fd, type, (void*)event_info))
	{
		m_TimerHeap.Remove((HeapItem*)event_info);
		m_EventInfoPool.Recycle((void*)event_info);
		LOG4CPLUS_ERROR(logger, "add_event failed. fd="<<fd<<" type="<<type);
		event_info = NULL;
	}

	return event_info!=NULL?true:false;
}

}//namespace

