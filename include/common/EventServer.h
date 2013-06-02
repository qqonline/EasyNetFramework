/*
 * EventHandler.h
 *
 *  Created on: 2013-4-15
 *      Author: LiuYongJin
 */

#ifndef _COMMON_EVENT_SERVER_H_
#define _COMMON_EVENT_SERVER_H_
#include <stdint.h>

namespace easynet
{


/////////////////////////////////////////////////////////////////////////////////////////
//事件类型
typedef uint16_t EventType;
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
#define ET_IS_PERSIST(x)    (((x)&ET_PERSIST) != 0)   //是否设置持续

static const char* EventInfoString[4]={"ET_EMPTY", "ET_READ", "ET_WRITE", "ET_RDWT"};
#define EventStr(x) EventInfoString[(x&ET_RDWT)]
/////////////////////////////////////////////////////////////////////////////////////////

typedef enum _handle_result_
{
	HANDLE_SUCC,
	HANDLE_ERROR,
	HANDLE_PEER_CLOSE,
}HANDLE_RESULT;

typedef enum _error_code_
{
	CODE_ERROR,
	CODE_TIMEOUT,
	CODE_PEER_CLOSE,
}ErrorCode;

class IEventHandler
{
public:
	virtual ~IEventHandler(){}
	//时钟超时
	virtual void OnTimeout(uint64_t nowtime_ms)=0;
	//错误事件
	virtual void OnEventError(int32_t fd, uint64_t nowtime_ms, ErrorCode code)=0;
	//可读事件
	virtual HANDLE_RESULT OnEventRead(int32_t fd, uint64_t nowtime_ms)=0;
	//可写事件
	virtual HANDLE_RESULT OnEventWrite(int32_t fd, uint64_t nowtime_ms)=0;
};

/** 事件监听server
 *  1. 监听时钟事件
 *  2. 监听IO读/写/超时事件
 */
class IEventServer
{
public:
	IEventServer():m_CanStop(false){}
	virtual ~IEventServer(){}

	/**添加定时器:
	 * @param handler    : 定时器事件的处理接口;
	 * @param timeout_ms : 定时器超时的时间.单位毫秒;
	 * @param persist    : true持续性定时器,每隔tiemout触发一次超时事件;false一次性定时器;
	 * @return           : true成功;false失败;
	 */
	virtual bool AddTimer(IEventHandler *handler, uint32_t timeout_ms, bool persist)=0;

	/**添加事件:
	 * @param fd         : socket描述符;
	 * @param type       : 待监听的事件.定义见<事件类型>;
	 * @param handler    : fd事件的处理接口;
	 * @param timeout_ms : fd容许的读写空闲时间,超过该时间没有发生读写事件将产生超时事件.
	 *                     小于0表示永不超时.单位毫秒;
	 * @return           : true成功;false失败;
	*/
	virtual bool AddEvent(int32_t fd, EventType type, IEventHandler *handler, int32_t timeout_ms)=0;

	/**删除事件:
	 * @param fd      : socket描述符;
	 * @param type    : type待删除的事件.定义见<事件类型>
	 * @return        : true成功;false失败;
	*/
	virtual bool DelEvent(int32_t fd, EventType type)=0;

	/** 分派事件
	 * @return        : true成功;false失败
	 */
	virtual bool DispatchEvents()=0;

	//循环分派事件
	void RunLoop()
	{
		while(!m_CanStop)
		{
			if(!DispatchEvents())
				break;
		}
		m_CanStop = false;
	}

	//停止分派事件
	void Stop(){m_CanStop = true;}
private:
	bool m_CanStop;
};


}//namespace

#endif //_COMMON_EVENT_SERVER_H_
