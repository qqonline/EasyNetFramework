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
using std::list;

class IODemuxer
{
public:
	IODemuxer(bool thread_safe);
	virtual ~IODemuxer();

	//添加定时器:
	//  handler:时钟超时事件处理句柄类;
	//  timeout:超时时间(单位:毫秒).当超时时,handler将被调用;
	//  persist:true每隔timeout产生一次超时事件;false只产生一次超时事件;
	bool add_timer(TimerHandler *handler, uint32_t timeout, bool persist=true);

private:
	Heap m_timer_heap;
	list<void*> m_timer_timeout_list;
	ObjectPool m_timerinfo_pool;
	pthread_mutex_t *m_timer_lock;
public:
	bool run_loop();
	void set_exit(){m_exit = true;}
private:
	bool m_exit;

public:
	//添加io事件:
	//  handler:io事件处理句柄类;
	//  fd:socket描述符;
	//  type:等待发生的事件.当事件产生时,handler将被调用;
	//  timeout:超时时间(单位:毫秒).当fd在timeout时间内没有产生读/写事件时,将产生超时事件,handler将被调用;
	virtual bool add_event(EventHandler *handler, uint32_t fd, EventType type, uint32_t timeout)=0;

	//删除fd上监听的type事件
	virtual bool delete_event(uint32_t fd, EventType type)=0;

protected:
	//分配发生的io事件
	//  wait_ms:等待事件发生的时间
	virtual void dispatch_events(uint32_t wait_ms)=0;
};

#endif //_COMMON_IODEMUXER_H_
