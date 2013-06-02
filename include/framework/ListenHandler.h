/*
 * ListenHandler.h
 *
 *  Created on: 2013-5-12
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_LISTEN_HANDLER_H_
#define _FRAMEWORK_LISTEN_HANDLER_H_
#include <assert.h>
#include <sys/socket.h>

#include "EventServer.h"
#include "Logger.h"

namespace easynet
{
class IAppInterface;
class ListenHandler:public IEventHandler
{
public:
	ListenHandler(IAppInterface *app_interface)
		:m_AppInterface(app_interface)
	{
		assert(m_AppInterface != NULL);
	}

//基类接口方法
public:
	//时钟超时
	void OnTimeout(uint64_t nowtime_ms);
	//错误事件
	void OnEventError(int32_t fd, uint64_t nowtime_ms, ErrorCode code);
	//可读事件
	HANDLE_RESULT OnEventRead(int32_t fd, uint64_t nowtime_ms);
	//可写事件
	HANDLE_RESULT OnEventWrite(int32_t fd, uint64_t nowtime_ms);
private:
	IAppInterface *m_AppInterface;
private:
	DECL_LOGGER(logger);
};

inline
void ListenHandler::OnTimeout(uint64_t nowtime_ms)
{
	assert(0);  //没有时钟
}

inline
HANDLE_RESULT ListenHandler::OnEventWrite(int32_t fd, uint64_t nowtime_ms)
{
	assert(0);  //无写事件
	return HANDLE_SUCC;
}


}//easynet
#endif //_FRAMEWORK_LISTEN_HANDLER_H_


