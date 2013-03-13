/*
 * eventhandler.h
 *
 *  Created on: 2013-3-7
 *      Author: LiuYongJin
 */

#ifndef _COMMON_EVENT_HANDLER_H_
#define _COMMON_EVENT_HANDLER_H_

#include <stdint.h>

typedef uint32_t EventType;
#define ET_WRITE     0X1   //写事件
#define ET_READ      0X2   //读事件
#define ET_RDWT      0X3   //读写事件
#define ET_WTRD      0X3   //读写事件
/*注:
 *  1. 读事件一旦添加到iodemuxer中后,iodemuxer不会主动删除读事件,除非发生错误或者handler返回HANDLE_SUCCESS/HANDLE_ERROR;
 *  2. 写事件添加到iodemuxer后,一旦产生了写事件,写事件将被删除,除非handler返回HANDLE_CONTINUE;
 */

typedef enum
{
	HANDLE_FINISH,      //结束(相应事件将从iodemuxer删除)
	HANDLE_CONTINUE,    //继续(相应事件继续保留在iodemuxer进行监听)
	HANDLE_ERROR        //失败(相应事件将从iodemuxer删除并且on_fd_error将被调用)
}HANDLE_RESULT;

class TimerHandler
{
public:
	virtual ~TimerHandler(){}

	//返回值:
	//  1.HANDLE_FINISH:时钟只触发一次
	//  2.HANDLE_CONTINUE:每隔一段时间触发一次
	//  3.HANDLE_ERROR:等价HANDLE_SUCCESS
	virtual HANDLE_RESULT on_timer_timeout()=0;
};

class EventHandler
{
public:
	virtual ~EventHandler(){}

	//响应fd的可读事件
	//返回值:
	//  1.HANDLE_FINISH:完成读操作,读事件被删除,直到重新被添加;
	//  2.HANDLE_CONTINUE:继续等待可读事件;
	//  3.HANDLE_ERROR:读操作发送错误,fd上所有的事件将被删除,同时on_fd_error将被调用;
	virtual HANDLE_RESULT on_fd_readable(uint32_t fd)=0;

	//响应fd的可写事件
	//返回值:
	//  1.HANDLE_FINISH:完成写操作,写事件被删除,直到重新被添加;
	//  2.HANDLE_CONTINUE:继续等待可写事件;
	//  3.HANDLE_ERROR:写操作发送错误,fd上所有的事件将被删除,同时on_fd_error将被调用;
	virtual HANDLE_RESULT on_fd_writeable(uint32_t fd)=0;

	//响应fd超时事件
	//返回值:忽略;
	virtual HANDLE_RESULT on_fd_timeout(uint32_t fd)=0;

	//响应fd错误事件
	//返回值:忽略;
	virtual HANDLE_RESULT on_fd_error(uint32_t fd)=0;
};

#endif //_COMMON_EVENT_HANDLER_H_
