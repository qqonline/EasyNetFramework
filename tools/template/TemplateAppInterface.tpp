/*
 * TemplateAppInterface.cpp
 *
 *  Created on: _#CreateDate#_
 *      Author: _#AuthorName#_
 */

#include "TemplateAppInterface.h"

IMPL_LOGGER(TemplateAppInterface, logger);

bool TemplateAppInterface::Start()
{
	//Add Your Code Here
	
	return true;
}

int32_t TemplateAppInterface::GetSocketRecvTimeout()
{
	return -1;
}

int32_t TemplateAppInterface::GetSocketIdleTimeout()
{
	return 3000;
}

int32_t TemplateAppInterface::GetMaxConnections()
{
	return 1000;
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
	LOG_DEBUG(logger, "send protocol succ on fd="<<fd<<", info='"<<context->Info<<"'");
	
	return ;
}

void TemplateAppInterface::OnSendError(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_ERROR(logger, "send protocol failed on fd="<<fd<<", info='"<<context->Info<<"'");
	
	return ;
}

void TemplateAppInterface::OnSendTimeout(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_WARN(logger, "send protocol timeout on fd="<<fd<<", info='"<<context->Info<<"'");
	
	return ;
}

void TemplateAppInterface::OnSocketFinished(int32_t fd)
{
	//Add Your Code Here
	LOG_INFO(logger, "socket finished. fd="<<fd);
	
	//close it?
	//Socket::Close(fd);

	return ;
}
