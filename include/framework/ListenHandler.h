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
#include "IAppInterface.h"
#include "Logger.h"

namespace easynet
{
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
	bool OnTimeout(uint64_t now_time)
	{
		return true;
	}

	//io超时protocol_factory
	bool OnTimeout(int32_t fd, uint64_t now_time)
	{
		assert(0);    //不允许超时
		return true;
	}

	//可读事件
	bool OnEventRead(int32_t fd, uint64_t now_time);

	//可写事件
	bool onEventWrite(int32_t fd, uint64_t now_time)
	{
		return true;
	}

	//错误事件
	bool OnEventError(int32_t fd, uint64_t now_time)
	{
		return true;
	}
private:
	IAppInterface *m_AppInterface;
private:
	DECL_LOGGER(logger);
};

}//easynet
#endif //_FRAMEWORK_LISTEN_HANDLER_H_


