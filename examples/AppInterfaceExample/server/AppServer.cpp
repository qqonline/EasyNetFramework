#include "AppServer.h"

#include "KVDataDefine.h"

IMPL_LOGGER(AppServer, logger);

bool AppServer::Start()
{
	//Add Your Code Here
	uint32_t port = 10000;
	if(!Listen(port))
	{
		LOG_DEBUG(logger, "listen on port="<<port<<" failed.");
		return false;
	}
	return true;
}

bool AppServer::OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)
{
	//Add Your Code Here
	LOG_DEBUG(logger, "receive protocol on fd="<<fd<<" info="<<context->info);
	
	return true;
}

void AppServer::OnSendSucc(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_DEBUG(logger, "send protocol succ on fd="<<fd<<" info="<<context->info);
	
	return ;
}

void AppServer::OnSendError(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_ERROR(logger, "send protocol failed on fd="<<fd<<" info="<<context->info);
	
	return ;
}

void AppServer::OnSendTimeout(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_WARN(logger, "send protocol timeout on fd="<<fd<<" info="<<context->info);
	
	return ;
}

bool AppServer::OnSocketError(int32_t fd)
{
	//Add Your Code Here
	LOG_ERROR(logger, "socket error on fd="<<fd);
	
	return true;
}

bool AppServer::OnSocketTimeout(int32_t fd)
{
	//Add Your Code Here
	LOG_ERROR(logger, "socket timeout on fd="<<fd);
	
	return true;
}


int32_t AppServer::GetRecvTimeout()
{
	return -1;
}

int32_t AppServer::GetIdleTimeout()
{
	return 3000;
}
