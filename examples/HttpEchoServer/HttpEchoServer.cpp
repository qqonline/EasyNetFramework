/*
 * HttpEchoServer.cpp
 *
 *  Created on: 2013-06-24
 *      Author: tim
 */

#include "HttpEchoServer.h"

#include "Socket.h"
#include "HttpReqProtocolFactory.h"

IMPL_LOGGER(HttpEchoServer, logger);

HttpEchoServer::HttpEchoServer()
{
	m_ProtocolFactory = (IProtocolFactory*)new HttpReqProtocolFactory;
}

bool HttpEchoServer::Start()
{
	//Add Your Code Here
	if(Listen(8080) == false)
	{
		LOG_ERROR(logger, "listen on port=8080 error.");
		return false;
	}

	LOG_INFO(logger, "server listen on port=8080 succ.");

	IEventServer *event_server = GetEventServer();
	event_server->RunLoop();

	return true;
}

int32_t HttpEchoServer::GetSocketRecvTimeout()
{
	return 3000;   //接收时间3s超时
}

int32_t HttpEchoServer::GetSocketIdleTimeout()
{
	return 3000;
}

int32_t HttpEchoServer::GetMaxConnections()
{
	return 1000;
}

bool HttpEchoServer::OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)
{
	//Add Your Code Here
	LOG_INFO(logger, "receive protocol on fd="<<fd<<", content is:\n"<<context->bytebuffer->m_Buffer);
	return true;
}

void HttpEchoServer::OnSendSucc(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_DEBUG(logger, "send protocol succ on fd="<<fd<<", info='"<<context->Info<<"'");
	
	return ;
}

void HttpEchoServer::OnSendError(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_ERROR(logger, "send protocol failed on fd="<<fd<<", info='"<<context->Info<<"'");
	
	return ;
}

void HttpEchoServer::OnSendTimeout(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_WARN(logger, "send protocol timeout on fd="<<fd<<", info='"<<context->Info<<"'");
	
	return ;
}

void HttpEchoServer::OnSocketFinished(int32_t fd)
{
	//Add Your Code Here
	LOG_INFO(logger, "socket finished. close it. fd="<<fd);
	Socket::Close(fd);

	return ;
}

IProtocolFactory* HttpEchoServer::GetProtocolFactory()
{
	return m_ProtocolFactory;
}
