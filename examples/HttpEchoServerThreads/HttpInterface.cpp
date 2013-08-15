/*
 * HttpInterface.cpp
 *
 *  Created on: 2013-07-13
 *      Author: tim
 */

#include "HttpInterface.h"
#include "Socket.h"

IMPL_LOGGER(HttpInterface, logger);

bool HttpInterface::Start()
{
	if(Listen(8080) == -1)
	{
		LOG_ERROR(logger, "listen on port=8080 error.");
		return false;
	}

	LOG_INFO(logger, "server listen on port=8080 succ.");

	//启动线程
	m_Index = 0;
	Thread *thread = NULL;
	thread = &m_HttpEchoServer[0];
	thread->Start();
	thread = &m_HttpEchoServer[1];
	thread->Start();

	IEventServer *event_server = GetEventServer();
	event_server->RunLoop();
	
	return true;
}

int32_t HttpInterface::GetSocketRecvTimeout()
{
	return -1;
}

int32_t HttpInterface::GetSocketIdleTimeout()
{
	return 3000;
}

int32_t HttpInterface::GetMaxConnections()
{
	return 1000;
}

bool HttpInterface::OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)
{
	//Add Your Code Here
	LOG_DEBUG(logger, "receive protocol on fd="<<fd);
	
	return true;
}

void HttpInterface::OnSendSucc(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_DEBUG(logger, "send protocol succ on fd="<<fd<<", info='"<<context->Info<<"'");
	
	return ;
}

void HttpInterface::OnSendError(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_ERROR(logger, "send protocol failed on fd="<<fd<<", info='"<<context->Info<<"'");
	
	return ;
}

void HttpInterface::OnSendTimeout(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_WARN(logger, "send protocol timeout on fd="<<fd<<", info='"<<context->Info<<"'");
	
	return ;
}

void HttpInterface::OnSocketFinished(int32_t fd)
{
	//Add Your Code Here
	LOG_INFO(logger, "socket finished. fd="<<fd);
	
	//close it?
	Socket::Close(fd);

	return ;
}

bool HttpInterface::AcceptNewConnect(int32_t fd)
{
	LOG_INFO(logger, "send fd="<<fd<<" to Index="<<m_Index);
	m_HttpEchoServer[m_Index].SendMessage(fd);
	m_Index = (m_Index+1)%2;
	return true;
}
