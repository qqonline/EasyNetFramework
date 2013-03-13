/*
 * eventhandler.h
 *
 *  Created on: 2013-3-7
 *      Author: LiuYongJin
 */

#ifndef _COMMON_EVENT_HANDLER_H_
#define _COMMON_EVENT_HANDLER_H_

#include <stdint.h>

typedef int EventType;
#define WRITE     1   //写事件
#define READ      2   //读事件
#define RDWT      3   //读写事件
#define WTRD      3   //读写事件
/*注:
 *  1. 读事件一旦添加到iodemuxer中后,iodemuxer不会主动删除读事件,除非发生错误或者handler返回HANDLE_SUCCESS/HANDLE_ERROR;
 *  2. 写事件添加到iodemuxer后,一旦产生了写事件,写事件将被删除,除非handler返回HANDLE_CONTINUE;
 */

typedef enum
{
	HANDLE_SUCCESS,     //成功(相应事件将从iodemuxer删除)
	HANDLE_CONTINUE,    //继续(相应事件继续保留在iodemuxer进行监听)
	HANDLE_ERROR        //失败(相应事件将从iodemuxer删除并且on_fd_error将被调用)
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
