/*
 * EventHandler.h
 *
 *  Created on: 2013-4-15
 *      Author: LiuYongJin
 */

#ifndef _COMMON_EVENT_SERVER_H_
#define _COMMON_EVENT_SERVER_H_

#include <stdint.h>

#include <common/Heap.h>
#include <common/ObjectPool.h>
#include <common/Logger.h>

namespace easynet
{


/////////////////////////////////////////////////////////////////////////////////////////
//事件类型
typedef uint8_t EventType;
#define EV_READ             0x0001                    //读
#define EV_WRITE            0x0002                    //写
#define EV_PERSIST          0x1000                    //持续,只对EV_READ有效

#define EV_RDWT             (EV_READ|EV_WRITE)        //读写
#define EV_WTRD             (EV_READ|EV_WRITE)        //读写
#define EV_PER_RD           (EV_READ|EV_PERSIST)      //持续读

#define EV_IS_EMPTY(x)      (((x)&EV_RDWT) == 0)      //是否为空
#define EV_IS_READ(x)       (((x)&EV_READ) != 0)      //是否设置读
#define EV_IS_WRITE(x)      (((x)&EV_WRITE) != 0)     //是否设置写
#define EV_IS_PERSIST(x)    (((x)&EV_PERSIT)!= 0)     //是否设置持续
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

/** 事件监听server
 *  1. 监听时钟事件
 *  2. 监听IO读/写/超时事件
 */
class EventServer
{
public:
	EventServer();
	virtual ~EventServer(){}

	/**添加定时器:
	 * @param handler    : 定时器事件的处理接口;
	 * @param timeout_ms : 定时器超时的时间.单位毫秒;
	 * @param persist    : true持续性定时器,每隔tiemout_ms处罚一次超时事件;false一次性定时器;默认true;
	 * @return           : true成功;false失败;
	 */
	bool AddTimer(EventHandler *handler, uint32_t timeout_ms, bool persist=true);

	/**添加事件:
	 * @param fd         : socket描述符;
	 * @param type       : 待监听的事件.定义见<事件类型>;
	 * @param handler    : fd事件的处理接口;
	 * @param timeout_ms : fd容许的读写空闲时间,超过该时间没有发生读写事件将产生超时事件.单位毫秒;
	 * @return           : true成功;false失败;
	*/
	bool AddEvent(int32_t fd, EventType type, EventHandler *handler, uint32_t timeout_ms);

	/**删除事件:
	 * @param fd         : socket描述符;
	 * @param type       : type待删除的事件.定义见<事件类型>
	 * @return            : true成功;false失败;
	*/
	bool DelEvent(int32_t fd, EventType type);
	
	//分派一轮事件
	bool DispatchEvents();

	//循环分派事件
	bool DispatchEvents_RunLoop();

	//停止循环分派事件
	void Stop();
private:
	bool m_CanStop;
	Heap m_TimerHeap;
	ObjectPool m_EventInfoPool;

////////////////////////////////////////////////////////////////////////
////////////////////////  派生类需要实现的接口  ////////////////////////
protected:
	virtual bool AddEvent(int32_t fd, EventType type, void *arg)=0;
	virtual bool NotifyEvent(int32_t fd, EventType type, void *arg)=0;
	virtual bool DelEvent(int32_t fd , EventType type)=0;
private:
	DECL_LOGGER(logger);
};

inline
bool EventServer::DispatchEvents_RunLoop()
{
	bool is_error = false;
	while(!m_CanStop && !is_error)
		is_error = DispatchEvents();
	return is_error;
}

inline
void EventServer::Stop()
{
	m_CanStop = true;
}



}//namespace

#endif //_COMMON_EVENT_SERVER_H_
