/*
 * EventHandler.h
 *
 *  Created on: 2013-4-15
 *      Author: LiuYongJin
 */

#ifndef _COMMON_EVENT_SERVER_H_
#define _COMMON_EVENT_SERVER_H_

#include <stdint.h>
#include <map>
using std::map;

#include <common/Heap.h>
#include <common/ArrayObjectPool.h>
#include <common/Logger.h>

namespace easynet
{


/////////////////////////////////////////////////////////////////////////////////////////
//事件类型
typedef uint8_t EventType;
#define ET_EMPTY            0x0000                    //空
#define ET_READ             0x0001                    //读
#define ET_WRITE            0x0002                    //写
#define ET_PERSIST          0x1000                    //持续,只对ET_READ有效

#define ET_RDWT             (ET_READ|ET_WRITE)        //读写
#define ET_WTRD             (ET_READ|ET_WRITE)        //读写
#define ET_PER_RD           (ET_READ|ET_PERSIST)      //持续读

#define ET_IS_EMPTY(x)      (((x)&ET_RDWT) == 0)      //是否为空
#define ET_IS_READ(x)       (((x)&ET_READ) != 0)      //是否设置读
#define ET_IS_WRITE(x)      (((x)&ET_WRITE) != 0)     //是否设置写
#define ET_IS_PERSIST(x)    (((x)&ET_PERSIT)!= 0)     //是否设置持续
/////////////////////////////////////////////////////////////////////////////////////////

class EventHandler
{
public:
	virtual ~EventHandler(){}
	virtual bool OnTimeout()=0;
	virtual bool OnEventRead(int32_t fd)=0;
	virtual bool onEventWrite(int32_t fd)=0;
	virtual bool OnEventError(int32_t fd)=0;
};

typedef struct _event_info
{
	int32_t fd;
	EventType type;
	EventHandler *handler;
	uint32_t timeout;
}EventInfo;

typedef map<uint32_t, EventInfo*> EventMap;

/** 事件监听server
 *  1. 监听时钟事件
 *  2. 监听IO读/写/超时事件
 */
class EventServer
{
public:
	EventServer(uint32_t max_events);
	virtual ~EventServer(){}

	/**添加定时器:
	 * @param handler : 定时器事件的处理接口;
	 * @param timeout : 定时器超时的时间.单位秒;
	 * @param persist : true持续性定时器,每隔tiemout触发一次超时事件;false一次性定时器;默认true;
	 * @return        : true成功;false失败;
	 */
	bool AddTimer(EventHandler *handler, uint32_t timeout, bool persist=true);

	/**添加事件:
	 * @param fd      : socket描述符;
	 * @param type    : 待监听的事件.定义见<事件类型>;
	 * @param handler : fd事件的处理接口;
	 * @param timeout : fd容许的读写空闲时间,超过该时间没有发生读写事件将产生超时事件.
	 *                  小于0表示永不超时.单位秒;
	 * @return        : true成功;false失败;
	*/
	bool AddEvent(int32_t fd, EventType type, EventHandler *handler, int32_t timeout);

	/**删除事件:
	 * @param fd      : socket描述符;
	 * @param type    : type待删除的事件.定义见<事件类型>
	 * @return        : true成功;false失败;
	*/
	bool DelEvent(int32_t fd, EventType type);

	//循环分派事件
	bool RunLoop();

	//停止分派事件
	void Stop(){m_CanStop = true;}

protected:
	bool m_CanStop;
	uint32_t m_MaxEvents;
	Heap m_TimerHeap;
	EventMap m_EventMap;
	ArrayObjectPool m_ObjectPool;

////////////////////////////////////////////////////////////////////////
////////////////////////  派生类需要实现的接口  ////////////////////////
protected:
	virtual bool AddEvent(int32_t fd, EventType type, EventInfo *event_info)=0;
	virtual bool ModifyEvent(int32_t fd, EventType type)=0;
	virtual bool DelEvent(int32_t fd)=0;


private:
	DECL_LOGGER(logger);
};


}//namespace

#endif //_COMMON_EVENT_SERVER_H_
