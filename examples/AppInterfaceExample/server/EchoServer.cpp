/*
 * EchoServer.cpp
 *
 *  Created on: 2013-05-31
 *      Author: tim
 */

#include "EchoServer.h"
#include "KVData.h"

#include "KVDataIndex.h"

IMPL_LOGGER(EchoServer, logger);

bool EchoServer::Start()
{
	//Add Your Code Here
	bool ret = Listen(12300);
	if(ret == false)
	{
		LOG_ERROR(logger, "listen on port=12300 failed.");
		return false;
	}

	LOG_INFO(logger, "listen on port=12300 succ.");

	IEventServer* event_server = GetEventServer();
	event_server->RunLoop();

	return true;
}

bool EchoServer::OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)
{
	//Add Your Code Here
	KVData *kvdata = (KVData*)context->protocol;
	int32_t protocol_type;
	if(!kvdata->GetValue(Index_ProtocolType, protocol_type))
	{
		LOG_ERROR(logger, "receive protocol on fd="<<fd<<". get protocol_type failed");
		return true;
	}

	LOG_INFO(logger, "receive protocol on fd="<<fd<<" protocol_type="<<protocol_type);
	if(protocol_type == 0)
	{
		int32_t client_id=-1;
		string client_string="error info";
		kvdata->GetValue(Index_ClientID, client_id);
		kvdata->GetValue(Index_ClientString, client_string);

		LOG_INFO(logger, "receive client info:client_id="<<client_id<<" client_string="<<client_string);
	}
	else
	{
		LOG_WARN(logger, "unknow protocol_type="<<protocol_type);
	}
	
	return true;
}

void EchoServer::OnSendSucc(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_DEBUG(logger, "send protocol succ on fd="<<fd);
	
	return ;
}

void EchoServer::OnSendError(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_ERROR(logger, "send protocol failed on fd="<<fd);
	
	return ;
}

void EchoServer::OnSendTimeout(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_WARN(logger, "send protocol timeout on fd="<<fd);
	
	return ;
}

bool EchoServer::OnSocketError(int32_t fd)
{
	//Add Your Code Here
	LOG_ERROR(logger, "socket error on fd="<<fd);
	
	return true;
}

bool EchoServer::OnSocketTimeout(int32_t fd)
{
	//Add Your Code Here
	LOG_ERROR(logger, "socket timeout on fd="<<fd);
	
	return true;
}


int32_t EchoServer::GetRecvTimeout()
{
	return -1;
}

int32_t EchoServer::GetIdleTimeout()
{
	return 3000;
}
