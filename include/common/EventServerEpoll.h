/*
 * EventServerEpoll.h
 *
 *  Created on: Apr 25, 2013
 *      Author: LiuYongJi
 */
#ifndef _COMMON_EVENTSERVER_EPOLL_H_
#define _COMMON_EVENTSERVER_EPOLL_H_
#include <map>
using std::map;

#include "EventServer.h"
#include "Heap.h"
#include "ArrayObjectPool.h"
#include "Logger.h"

namespace easynet
{

typedef map<int32_t, void*> FDMap;

class EventServerEpoll:public IEventServer
{
public:
	EventServerEpoll(uint32_t max_events);
	~EventServerEpoll();

public:  //实现接口方法
	bool AddTimer(EventHandler *handler, uint32_t timeout, bool persist);
	bool AddEvent(int32_t fd, EventType type, EventHandler *handler, int32_t timeout);
	bool DelEvent(int32_t fd, EventType type);
	bool DispatchEvents();

private:
	int32_t         m_EpFd;
	int32_t         m_MaxEvents;
	void*           m_EventData;
	Heap            m_TimerHeap;
	FDMap           m_FDMap;
	ArrayObjectPool m_ObjectPool;

private:
	DECL_LOGGER(logger);
};


}//namespace
#endif //_COMMON_EVENTSERVER_EPOLL_H_
