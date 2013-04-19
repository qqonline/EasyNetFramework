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
	uint32_t timeout;
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

EventServer::EventServer()
	:m_TimerHeap(_event_info_cmp)
	,m_EventInfoPool(sizeof(EventInfo))
	,m_CanStop(false)
{
}

bool EventServer::_AddTimer(int32_t fd, EventHandler *handler, int32_t timeout_ms)
{
	assert(handler!=NULL && timeout>=0);

	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t now_time = tv.tv_sec*1000+tv.tv_usec/1000;

	TimerInfo *timer_info;
	if((timer_info=(TimerInfo*)m_timerinfo_pool.Get()) == NULL)
	{
		LOG4CPLUS_WARN(logger, "timerinfo_poll out of memory.");
		return false;
	}
	timer_info->heap_item.index = -1;
	timer_info->handler = handler;
	timer_info->timeout = timeout;
	timer_info->expire_time = now_time+timeout;
	if(!m_timer_heap.insert((HeapItem*)timer_info))
	{
		m_timerinfo_pool.recycle((void*)timer_info);
		LOG4CPLUS_WARN(logger, "insert timerinfo into timer_heap failed.");
		return false;
	}

	LOG4CPLUS_DEBUG(logger, "insert timer success. timeout="<<timeout);
	return true;
}


}//namespace

