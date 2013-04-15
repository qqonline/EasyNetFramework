/*
 * IODemuxer.cpp
 *
 *  Created on: 2013-3-7
 *      Author: LiuYongJin
 */

#include <common/IODemuxer.h>
#include <common/Logger.h>

#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>

#include <list>
using std::list;

namespace easynet
{

IMPL_LOGGER(IODemuxer, logger);

#define WAIT_TIME    200    //io事件的等待时间取该值和最小超时时间的较小值

typedef struct _timer_info
{
	HeapItem heap_item;
	uint32_t timeout;
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

IODemuxer::IODemuxer()
	:m_timer_heap(_timer_info_cmp)
	,m_timerinfo_pool(sizeof(TimerInfo))
	,m_exit(false)
{
}

bool IODemuxer::add_timer(TimerHandler *handler, int32_t timeout)
{
	assert(handler!=NULL && timeout>=0);

	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t now_time = tv.tv_sec*1000+tv.tv_usec/1000;

	TimerInfo *timer_info;
	if((timer_info=(TimerInfo*)m_timerinfo_pool.get()) == NULL)
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

bool IODemuxer::run_loop()
{

	uint64_t now_time;                //当前时间(单位ms)
	uint32_t wait_time;               //io事件等待时间(单位ms)
	list<TimerInfo*> timeout_list;    //超时时钟列表

	struct timeval tv;
	gettimeofday(&tv, NULL);
	now_time = tv.tv_sec*1000+tv.tv_usec/1000;

	m_exit = false;
	while(!m_exit)
	{
		wait_time = WAIT_TIME;

		//收集时钟超时事件,同时设置wait_time
		timeout_list.clear();
		TimerInfo *timer_info;
		while((timer_info=(TimerInfo*)m_timer_heap.top()) != NULL)
		{
			if(timer_info->expire_time <= now_time)    //时钟超时
			{
				timeout_list.push_back(timer_info);
				m_timer_heap.pop();
				continue;
			}

			if((wait_time=timer_info->expire_time-now_time) > WAIT_TIME)    //取最小值
				wait_time = WAIT_TIME;
			break;
		}

		//处理io事件
		dispatch_events(now_time, wait_time);

		//处理发生的时钟超时事件
		while(!timeout_list.empty())
		{
			TimerInfo *timer_info = *timeout_list.begin();
			timeout_list.pop_front();

			if(timer_info->handler->on_timer_timeout() == HANDLE_CONTINUE)    //间隔时钟
			{
				timer_info->expire_time = now_time+timer_info->timeout;
				bool temp = m_timer_heap.insert((HeapItem*)timer_info);
				assert(temp == true);
			}
			else
			{
				m_timerinfo_pool.recycle((void*)timer_info);
				LOG4CPLUS_DEBUG(logger, "timer finished, remove from timer_heap.");
			}
		}

		gettimeofday(&tv, NULL);
		uint64_t temp = tv.tv_sec*1000+tv.tv_usec/1000;
		if(temp-now_time >= 500)
		{
			LOG4CPLUS_WARN(logger, "one loop using times(ms):"<<temp-now_time);
		}
		now_time = temp;
	}

	return true;
}


}//namespace
