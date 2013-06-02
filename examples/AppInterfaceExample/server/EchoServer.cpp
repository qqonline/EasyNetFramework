/*
 * EchoServer.cpp
 *
 *  Created on: 2013-05-31
 *      Author: tim
 */

#include "EchoServer.h"
#include "KVData.h"
#include "Socket.h"
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

		IProtocolFactory* protocol_factory = GetProtocolFactory();
		assert(protocol_factory != NULL);
		ProtocolContext *send_context = NewProtocolContext();
		ByteBuffer *bytebuffer = send_context->bytebuffer;

		uint32_t header_size = protocol_factory->HeaderSize();
		//预留协议头
		bytebuffer->m_Size = header_size;

		//协议体数据
		int32_t protocol_server = 1;
		int32_t server_id = 1001;
		const char* server_string = "I'am a server";

		KVData kvdata;
		kvdata.AttachWriteBuffer(bytebuffer, true);
		kvdata.SetValue(Index_ProtocolType, protocol_server);
		kvdata.SetValue(Index_ServerID, server_id);
		kvdata.SetValue(Index_ServerString, server_string);

		uint32_t body_size = bytebuffer->m_Size-header_size;
		protocol_factory->EncodeHeader(bytebuffer->m_Buffer, body_size);

		send_context->Info = "ServerInfo";
		SendProtocol(fd, send_context, 300000);
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
	LOG_DEBUG(logger, "send protocol succ on fd="<<fd<<", info='"<<context->Info<<"'");
	DeleteProtocolContext(context);
	return ;
}

void EchoServer::OnSendError(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_ERROR(logger, "send protocol failed on fd="<<fd<<", info='"<<context->Info<<"'");
	DeleteProtocolContext(context);
	return ;
}

void EchoServer::OnSendTimeout(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_WARN(logger, "send protocol timeout on fd="<<fd<<", info='"<<context->Info<<"'");
	DeleteProtocolContext(context);
	return ;
}

void EchoServer::OnSocketFinished(int32_t fd)
{
	//Add Your Code Here
	LOG_INFO(logger, "socket finished and close it. fd="<<fd);
	Socket::Close(fd);
	return ;
}

int32_t EchoServer::GetRecvTimeoutMS()
{
	return -1;
}

int32_t EchoServer::GetIdleTimeoutMS()
{
	return 300000;
}
