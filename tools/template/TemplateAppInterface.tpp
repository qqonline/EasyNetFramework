#include "TemplateAppInterface.h"

IMPL_LOGGER(TemplateAppInterface, logger);

bool TemplateAppInterface::Start()
{
	//Add Your Code Here
	
	return true;
}

bool TemplateAppInterface::OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)
{
	//Add Your Code Here
	LOG_DEBUG(logger, "receive protocol on fd="<<fd);
	
	return true;
}

void TemplateAppInterface::OnSendSucc(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_DEBUG(logger, "send protocol succ on fd="<<fd);
	
	return ;
}

void TemplateAppInterface::OnSendError(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_ERROR(logger, "send protocol failed on fd="<<fd);
	
	return ;
}

void TemplateAppInterface::OnSendTimeout(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_WARN(logger, "send protocol timeout on fd="<<fd);
	
	return ;
}

bool TemplateAppInterface::OnSocketError(int32_t fd)
{
	//Add Your Code Here
	LOG_ERROR(logger, "socket error on fd="<<fd);
	
	return true;
}

bool TemplateAppInterface::OnSocketTimeout(int32_t fd)
{
	//Add Your Code Here
	LOG_ERROR(logger, "socket timeout on fd="<<fd);
	
	return true;
}


int32_t TemplateAppInterface::GetRecvTimeout()
{
	return -1;
}

int32_t TemplateAppInterface::GetIdleTimeout()
{
	return 3000;
}
