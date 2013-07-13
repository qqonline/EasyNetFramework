/*
 * PipeHandler.cpp
 *
 *  Created on: 2013-7-13
 *      Author: LiuYongJin
 */
#include <unistd.h>
#include "MessageHandler.h"
#include "IAppInterface.h"

namespace easynet
{

//时钟超时
void MessageHandler::OnTimeout(uint64_t now_time)
{
	assert(0);
}

//错误事件
void MessageHandler::OnEventError(int32_t fd, uint64_t nowtime_ms, ERROR_CODE code)
{
	assert(0);
}

//可读事件
ERROR_CODE MessageHandler::OnEventRead(int32_t fd, uint64_t now_time)
{
	int32_t msg;
	while(true)
	{
		if(read(fd, &msg, sizeof(msg)) == -1)
			break;
		bool ret = m_AppInterface->OnRecvMessage(msg);
		assert(ret == true);
	}
	return ECODE_SUCC;
}

ERROR_CODE MessageHandler::OnEventWrite(int32_t fd, uint64_t now_time)
{
	assert(0);
	return ECODE_SUCC;  //无可写事件
}


}//namespace
