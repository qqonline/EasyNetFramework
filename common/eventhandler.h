/*
 * eventhandler.h
 *
 *  Created on: 2013-3-7
 *      Author: LiuYongJin
 */

#define _COMMON_EVENT_HANDLER_H_
#define _COMMON_EVENT_HANDLER_H_

typedef int EventType;
#define WRITE     1   //写事件
#define READ      2   //读事件
#define PERSIST   4   //事件的持续标记
#define RDWT      3   //读写事件
#define WTRD      3   //读写事件
#define RDPE      6   //持续读事件
#define WTRDPE    7   //持续读事件和写事件
////注:没有持续写事件.等待可写需要根据实际情况设置;

typedef enum
{
	HANDLE_SUCCESS,
	HANDLE_ERROR
}HANDLE_RESULT;

class TimerHandler
{
public:
	virtual ~TimerHandler(){}
	virtual HANDLE_RESULT on_timer_timeout()=0;
};

class EventHandler
{
public:
	virtual ~EventHandler(){}
	virtual HANDLE_RESULT on_fd_readable(uint32_t fd)=0;
	virtual HANDLE_RESULT on_fd_writeable(uint32_t fd)=0;
	virtual HANDLE_RESULT on_fd_error(uint32_t fd, int errno)=0;
	virtual HANDLE_RESULT on_fd_timeout(uint32_t fd)=0;
};

#endif //_COMMON_EVENT_HANDLER_H_
