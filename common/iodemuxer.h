/*
 * iodemuxer.h
 *
 *  Created on: 2013-3-7
 *      Author: LiuYongJin
 */

#ifndef _COMMON_IODEMUXER_H_
#define _COMMON_IODEMUXER_H_

#include <stdint.h>
#include <pthread.h>

#include "eventhandler.h"
#include "heap.h"
#include "objectpool.h"

#include <list>
#include <map>
using std::list;
using std::map;

//event flags
#define EVENT_FLAG_ERROR     1
#define EVENT_FLAG_WRITE     2
#define EVENT_FLAG_READ      4
typedef struct _event_info_
{
	uint32_t fd;
	int event_flags;       //发生的事件
}OccurEvent;

typedef list<OccurEvent> EventList;
typedef map<uint32_t, void*> EventInfoMap;

class IODemuxer
{
public:
	IODemuxer(bool thread_safe=true);
	virtual ~IODemuxer();

	//添加定时器:
	//  handler:时钟超时事件处理句柄类;
	//  timeout:超时时间(单位:毫秒).当超时时,handler将被调用;
	//  persist:true每隔timeout产生一次超时事件;false只产生一次超时事件;
	bool add_timer(TimerHandler *handler, uint32_t timeout, bool persist=true);

	//添加io事件:
	//  handler:io事件处理句柄类;
	//  fd:socket描述符;
	//  type:等待发生的事件.当事件产生时,handler将被调用;
	//  timeout:超时时间(单位:毫秒).当fd在timeout时间内没有产生读/写事件时,将产生超时事件,handler将被调用;
	bool add_event(EventHandler *handler, uint32_t fd, EventType type, uint32_t timeout=3000);


	bool run_loop();
	void set_exit(){m_exit = true;}
protected:
	//等待io事件发生
	//event_list:发生事件的列表
	//wait_ms:等待时间(单位ms)
	//返回:true成功;false出错
	virtual bool wait_event(EventList &event_list, uint32_t wait_ms)=0;
	virtual bool add_event(uint32_t fd, EventType type)=0;
	virtual bool delete_event(uint32_t fd, EventType type)=0;
private:
	EventInfoMap m_eventinfo_map;
	list<EventInfo*> m_event_timeout_list;

	Heap m_timer_heap;
	ObjectPool m_timerinfo_pool;
	list<TimerInfo*> m_timer_timeout_list;

	bool m_exit;

	pthread_mutex_t *m_event_lock;
	pthread_mutex_t *m_timer_lock;
};

#endif //_COMMON_IODEMUXER_H_
