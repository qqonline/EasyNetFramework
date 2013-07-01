/*
 * EventServerEpoll.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: LiuYongJin
 */
#include <sys/epoll.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <list>
using std::list;

#include "EventServerEpoll.h"

namespace easynet
{

IMPL_LOGGER(EventServerEpoll, logger);
#define WAIT_TIME    200    //io事件的等待时间取该值和最小超时时间的较小值

typedef struct _event_info
{
	HeapItem       heap_item;
	int32_t        fd;
	EventType      type;
	IEventHandler  *handler;
	int32_t        timeout_ms;      //超时时间(单位毫秒)
	uint64_t       expire_time;    //超时时间点(单位毫秒)
}EventInfo;

//单位毫秒
#define GetCurTime(now) do{              \
	struct timeval tv;                    \
	gettimeofday(&tv, NULL);              \
	now = tv.tv_sec*1000+tv.tv_usec/1000; \
}while(0)

#define SetEventInfo(event_info, f, t, h, to) do{ \
	event_info->heap_item.index = -1;              \
	event_info->expire_time     = 0;               \
	event_info->fd              = f;               \
	event_info->type            = t;               \
	event_info->handler         = h;               \
	event_info->timeout_ms      = to;              \
}while(0)

#define SetTimerInfo(event_info,t_ms)         do{ \
	int64_t now;                                   \
	GetCurTime(now);                               \
	event_info->expire_time     = now+t_ms;        \
}while(0)

static int _timer_cmp(HeapItem *item0, HeapItem *item1)
{
	EventInfo *e0 = (EventInfo*)item0;
	EventInfo *e1 = (EventInfo*)item1;
	if(e0->expire_time < e1->expire_time)
		return -1;
	else if (e0->expire_time == e1->expire_time)
		return 0;
	else
		return 1;
}

EventServerEpoll::EventServerEpoll(uint32_t max_events)
	:m_ObjectPool(sizeof(EventInfo), max_events)
	,m_TimerHeap(_timer_cmp)
	,m_MaxEvents(max_events)
{
	m_EpFd = epoll_create1(EPOLL_CLOEXEC);
	assert(m_EpFd > 0);
	m_EventData = malloc(sizeof(struct epoll_event)*m_MaxEvents);
	assert(m_EventData != NULL);
}

EventServerEpoll::~EventServerEpoll()
{
	close(m_EpFd);
	free(m_EventData);
}

//////////////////////////  接口方法  //////////////////////////
//添加时钟
bool EventServerEpoll::AddTimer(IEventHandler *handler, uint32_t timeout_ms, bool persist)
{
	assert(handler != NULL);
	EventInfo *event_info = new EventInfo;
	EventType type = persist?ET_PERSIST:ET_EMPTY;

	SetEventInfo(event_info, -1, type, handler, timeout_ms);
	SetTimerInfo(event_info, timeout_ms);
	if(m_TimerHeap.Insert((HeapItem*)event_info))
		return true;

	//failed
	LOG_ERROR(logger, "add timer failed.");
	delete event_info;
	return false;
}

//添加IO事件
bool EventServerEpoll::AddEvent(int32_t fd, EventType type, IEventHandler *handler, int32_t timeout_ms)
{
	if(fd<0 || ET_IS_EMPTY(type) || handler==NULL)
	{
		LOG_WARN(logger, "add_event but parameters invalid. fd="<<fd<<", type="<<type<<"("<<EventStr(type)<<").");
		return false;
	}

	FDMap::iterator it = m_FDMap.find(fd);
    //新增事件
	if(it == m_FDMap.end())
	{
		EventInfo *event_info = (EventInfo *)m_ObjectPool.Get();
		if(event_info == NULL)
		{
			LOG_ERROR(logger, "out of memory. fd="<<fd<<", type="<<type<<"("<<EventStr(type)<<").");
			return false;
		}
		SetEventInfo(event_info, fd, type, handler, timeout_ms);

		//保存到event map
		std::pair<FDMap::iterator, bool> result = m_FDMap.insert(std::make_pair(fd, (void*)event_info));
		if(result.second == false)
		{
			LOG_ERROR(logger, "insert event_map failed. fd="<<fd<<", type="<<type<<"("<<EventStr(type)<<").");
			m_ObjectPool.Recycle((void*)event_info);
			return false;
		}

		//添加时钟
		if(timeout_ms >= 0)
		{
			SetTimerInfo(event_info, timeout_ms);
			if(!m_TimerHeap.Insert((HeapItem*)event_info))
			{
				LOG_ERROR(logger, "add timer failed. fd="<<fd<<", type="<<type<<"("<<EventStr(type)<<").");
				m_FDMap.erase(fd);
				m_ObjectPool.Recycle((void*)event_info);
				return false;
			}
		}

		//添加事件
		struct epoll_event ep_event;
		ep_event.events = 0;
		ep_event.data.ptr = (void*)event_info;
		if(ET_IS_READ(type))
			ep_event.events |= EPOLLIN;
		if(ET_IS_WRITE(type))
			ep_event.events |= EPOLLOUT;
		if(epoll_ctl(m_EpFd, EPOLL_CTL_ADD, fd, &ep_event) != 0)
		{
			LOG_ERROR(logger, "add event failed. fd="<<fd<<", type="<<type<<"("<<EventStr(type)<<")"<<", errno="<<errno<<"("<<strerror(errno)<<").");
			m_FDMap.erase(fd);
			if(timeout_ms >= 0)
				m_TimerHeap.Remove((HeapItem*)event_info);
			m_ObjectPool.Recycle((void*)event_info);
			return false;
		}

		return true;
	}

	//修改事件
	EventInfo *event_info = (EventInfo *)it->second;
	EventType old_type = event_info->type&ET_RDWT;
	EventType new_type = (event_info->type|type)&ET_RDWT;
	if(new_type == old_type)         //已经存在
	{
		LOG_DEBUG(logger, "new event_type is equal to the old. nothing to do.");
		event_info->type |= type;    //maybe EV_PERSIST
		return true;
	}
	new_type = event_info->type|type;

	struct epoll_event ep_event;
	ep_event.events = 0;
	ep_event.data.ptr = (void*)event_info;
	if(ET_IS_READ(new_type))
		ep_event.events |= EPOLLIN;
	if(ET_IS_WRITE(new_type))
		ep_event.events |= EPOLLOUT;
	if(epoll_ctl(m_EpFd, EPOLL_CTL_MOD, fd, &ep_event) != 0)
	{
		LOG_ERROR(logger, "modify event failed. fd="<<fd<<", old_type="<<event_info->type<<"("<<EventStr(event_info->type)<<")."<<", new_type="<<new_type<<"("<<EventStr(new_type)<<")"<<", errno="<<errno<<"("<<strerror(errno)<<").");
		return false;
	}

	event_info->type = new_type;
	return true;
}

//删除IO事件
bool EventServerEpoll::DelEvent(int32_t fd, EventType type)
{
	if(fd<0 || ET_IS_EMPTY(type))
	{
		LOG_DEBUG(logger, "delete event but parameters invalid. fd="<<fd<<", type="<<type);
		return true;
	}

	FDMap::iterator it = m_FDMap.find(fd);
	if(it == m_FDMap.end())
	{
		LOG_INFO(logger, "delete event but can't found in event_map. fd="<<fd<<", type="<<type<<"("<<EventStr(type)<<")");
		return true;
	}

	EventInfo *event_info = (EventInfo *)it->second;
	EventType new_type = (event_info->type&~type)&ET_RDWT;
	//删除事件
	if(new_type == 0)
	{
		if(epoll_ctl(m_EpFd, EPOLL_CTL_DEL, fd, NULL) != 0)
		{
			LOG_ERROR(logger, "delete event failed. fd="<<fd<<", errno=%d"<<errno<<"("<<strerror(errno)<<").");
			return false;
		}
		m_FDMap.erase(fd);
		if(event_info->timeout_ms >= 0)
			m_TimerHeap.Remove((HeapItem*)event_info);
		m_ObjectPool.Recycle((void*)event_info);
		return true;
	}

	//修改事件
	new_type = event_info->type&~type;
	struct epoll_event ep_event;
	ep_event.events = 0;
	ep_event.data.ptr = (void*)event_info;
	if(ET_IS_READ(new_type))
		ep_event.events |= EPOLLIN;
	if(ET_IS_WRITE(new_type))
		ep_event.events |= EPOLLOUT;
	if(epoll_ctl(m_EpFd, EPOLL_CTL_MOD, fd, &ep_event) != 0)
	{
		LOG_ERROR(logger, "modify event failed. fd="<<fd<<", old_type="<<event_info->type<<"("<<EventStr(event_info->type)<<")"<<", new_type="<<new_type<<"("<<EventStr(new_type)<<")"<<" errno=%d"<<errno<<"("<<strerror(errno)<<").");
		return false;
	}

	event_info->type = new_type;
	return true;
}

//分派时钟/IO事件
bool EventServerEpoll::DispatchEvents()
{
	uint64_t now;
	GetCurTime(now);
	uint32_t wait_time = WAIT_TIME;           //io事件等待时间(单位ms)

	list<EventInfo*> timeout_list;
	EventInfo *event_info = NULL;
	//收集时钟超时事件,同时设置wait_time
	while((event_info=(EventInfo*)m_TimerHeap.Top()) != NULL)
	{
		if(event_info->expire_time <= now)    //时钟超时
		{
			timeout_list.push_back(event_info);
			m_TimerHeap.Pop();
			if(event_info->fd > 0)            //删除超时fd的io事件
			{
				LOG_DEBUG(logger, "delete event because of timeout. fd="<<event_info->fd);
				if(epoll_ctl(m_EpFd, EPOLL_CTL_DEL, event_info->fd, NULL) != 0)
				{
					LOG_ERROR(logger, "delete event failed. fd="<<event_info->fd<<", errno=%d"<<errno<<"("<<strerror(errno)<<").");
				}
			}
			continue;
		}

		if((wait_time=event_info->expire_time-now) > WAIT_TIME)    //取最小值
			wait_time = WAIT_TIME;
		break;
	}

	//处理io事件
	int32_t event_count = 0;
	if((event_count=epoll_wait(m_EpFd, (struct epoll_event*)m_EventData, m_MaxEvents, wait_time)) == -1)
	{
		event_count = 0;
		if(errno != EINTR)
			LOG_ERROR(logger, "wait events error. errno="<<errno<<"("<<strerror(errno)<<").");
	}
	int32_t i;
	for(i=0; i<event_count; ++i)
	{
		struct epoll_event* ep_event = ((struct epoll_event*)m_EventData)+i;
		EventInfo *event_info = (EventInfo*)ep_event->data.ptr;
		LOG_DEBUG(logger, "io event occur. fd="<<event_info->fd<<", epoll_event="<<ep_event->events);

		bool no_error = true;
		ERROR_CODE error_code = ECODE_SUCC;
		EventType del_type = ET_EMPTY;
		if(ep_event->events & (EPOLLERR|EPOLLRDHUP|EPOLLHUP))    //发生错误
		{
			LOG_ERROR(logger, "error occur on fd="<<event_info->fd<<"errno="<<errno<<"("<<strerror(errno)<<").");
			error_code = ECODE_ERROR;
			no_error = false;
			del_type = ET_RDWT;
		}

		if(no_error && (ep_event->events&EPOLLIN))
		{
			IEventHandler *event_handler = event_info->handler;
			error_code = event_handler->OnEventRead(event_info->fd, now);
			if(error_code==ECODE_ERROR || error_code==ECODE_CLOSE)
				no_error = false;
			if(!no_error || (error_code!=ECODE_PENDING && !ET_IS_PERSIST(event_info->type)))    //去掉非持续读事件,pending不去掉
				del_type |= ET_READ;
		}

		if(no_error && (ep_event->events&EPOLLOUT))
		{
			error_code = event_info->handler->OnEventWrite(event_info->fd, now);
			if(error_code==ECODE_ERROR || error_code==ECODE_CLOSE)
				no_error = false;
			if(!no_error || error_code!=ECODE_PENDING)    //pending不去掉
				del_type |= ET_WRITE;    //直接去掉读事件
		}

		if(!ET_IS_EMPTY(del_type))
		{
			LOG_DEBUG(logger, "delete event from event server. fd="<<event_info->fd<<", erroc_code="<<error_code<<"("<<ErrCodeStr(error_code)<<"), del_type="<<del_type<<"("<<EventStr(del_type)<<").");
			DelEvent(event_info->fd, del_type);
			if(!no_error)
				event_info->handler->OnEventError(event_info->fd, now, error_code);
		}
	}

	uint64_t now1 = now;
	if(event_count > 0)
		GetCurTime(now1);
	if(now1-now > 500)   //超过500ms
		LOG_WARN(logger, "deal with IO events="<<event_count<<", cost_time="<<now1-now);
	else
		LOG_TRACE(logger, "deal with IO events="<<event_count<<", cost_time="<<now1-now);
	now = now1;

	//处理超时事件
	event_count = timeout_list.size();
	while(!timeout_list.empty())
	{
		EventInfo *event_info = timeout_list.front();
		timeout_list.pop_front();

		if(event_info->fd >= 0)    //io超时
		{
			event_info->handler->OnEventError(event_info->fd, now, ECODE_TIMEOUT);
			m_FDMap.erase(event_info->fd);
			m_ObjectPool.Recycle((void*)event_info);
		}
		else    //时钟超时
		{
			event_info->handler->OnTimeout(now);
			if(ET_IS_PERSIST(event_info->type))
			{
				event_info->heap_item.index = -1;
				event_info->expire_time = event_info->timeout_ms+now;
				if(m_TimerHeap.Insert((HeapItem*)event_info))
					continue;
				LOG_ERROR(logger, "add timer failed.");
			}
			delete event_info;
		}
	}

	if(event_count)
		GetCurTime(now1);
	if(now1-now > 500)   //超过500ms
		LOG_WARN(logger, "deal with timeout events="<<event_count<<", cost_time="<<now1-now);
	else
		LOG_TRACE(logger, "deal with timeout events="<<event_count<<", cost_time="<<now1-now);

	return true;
}


}//namespace
