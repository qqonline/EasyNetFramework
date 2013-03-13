/*
 * iodemuxer_epoll.h
 *
 *  Created on: 2013-3-11
 *      Author: LiuYongJin
 */

#ifndef _COMMON_IODEMUXER_EPOLL_H_
#define _COMMON_IODEMUXER_EPOLL_H_

#include "iodemuxer.h"
#include "sys/epoll.h"
#include "objectpool.h"

#include <stdint.h>
#include <map>
using std::map;

typedef map<uint32_t, void*> EventInfoMap;    //key:fd; value:event_info

class IODemuxerEpoll:public IODemuxer
{
public:
	IODemuxerEpoll(bool et_mode=false);
	~IODemuxerEpoll();

public:  //实现基类纯虚函数
	//添加io事件:
	bool add_event(uint32_t fd, EventType type, EventHandler *handler, uint32_t timeout);
	//删除fd上监听的type事件
	bool delete_event(uint32_t fd);
	//分配发生的io事件
	void dispatch_events(uint64_t now_ms, uint32_t wait_ms);
private:
	uint32_t m_epfd;
	ObjectPool m_eventinfo_pool;
	EventInfoMap m_eventinfo_map;
	Heap m_timeout_heap;    //fd超时heap
	void *m_event_lock;

	struct epoll_event *m_epoll_events;
	uint32_t m_size;

	bool m_et_mode;
};

#endif //_COMMON_IODEMUXER_EPOLL_H_
