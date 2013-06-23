/*
 * Timer.h
 *
 *  Created on: 2013-6-23
 *      Author: tim
 */

#ifndef _APP_TIMER_H_
#define _APP_TIMER_H_

#include "EventServer.h"
#include "EchoServer.h"
using namespace easynet;

class Timer:public IEventHandler
{
public:
	Timer(EchoServer *echo_server):m_EchoServer(echo_server){}

	//时钟超时
	void OnTimeout(uint64_t nowtime_ms);
	//错误事件
	void OnEventError(int32_t fd, uint64_t nowtime_ms, ERROR_CODE code){return ;}
	//可读事件
	virtual ERROR_CODE OnEventRead(int32_t fd, uint64_t nowtime_ms){return ECODE_SUCC;}
	//可写事件
	virtual ERROR_CODE OnEventWrite(int32_t fd, uint64_t nowtime_ms){return ECODE_SUCC;}
private:
	EchoServer *m_EchoServer;
};

inline
void Timer::OnTimeout(uint64_t nowtime_ms)
{
	m_EchoServer->OnTimeout(nowtime_ms);
}

#endif

