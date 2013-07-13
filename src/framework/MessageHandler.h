/*
 * PipeHandler.h
 *
 *  Created on: 2013-7-13
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_MESSAGE_HANDLER_H_
#define _FRAMEWORK_MESSAGE_HANDLER_H_

#include <stddef.h>
#include <assert.h>
#include "EventServer.h"

namespace easynet
{

class IAppInterface;
class MessageHandler:public IEventHandler
{
public:
	MessageHandler(IAppInterface *app_interface)
		:m_AppInterface(app_interface)
	{
		assert(m_AppInterface != NULL);
	}

//基类接口方法
public:
	//时钟超时
	void OnTimeout(uint64_t now_time);
	//错误事件
	void OnEventError(int32_t fd, uint64_t nowtime_ms, ERROR_CODE code);
	//可读事件
	ERROR_CODE OnEventRead(int32_t fd, uint64_t now_time);
	//可写事件
	ERROR_CODE OnEventWrite(int32_t fd, uint64_t now_time);
private:
	IAppInterface *m_AppInterface;
};

}//namespace

#endif //_FRAMEWORK_MESSAGE_HANDLER_H_

