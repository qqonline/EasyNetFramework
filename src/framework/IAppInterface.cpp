/*
 * IAppInterface.cpp
 *
 *  Created on: 2013-5-9
 *      Author: LiuYongJin
 */
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "IAppInterface.h"
#include "EventServerEpoll.h"
#include "KVDataProtocolFactory.h"
#include "Socket.h"

namespace easynet
{
IMPL_LOGGER(IAppInterface, logger);

#define GetCurTime(now) do{                  \
	struct timeval tv;                        \
	gettimeofday(&tv, NULL);                  \
	now = tv.tv_sec*1000+tv.tv_usec/1000;     \
}while(0)

IAppInterface::IAppInterface()
	:m_EventServer(NULL)
	,m_ProtocolFactory(NULL)
	,m_TransHandler(NULL)
{}

IAppInterface::~IAppInterface()
{
	if(m_EventServer != NULL)
		delete m_EventServer;
	if(m_ProtocolFactory != NULL)
		delete m_ProtocolFactory;
	if(m_TransHandler != NULL)
		delete m_TransHandler;
	if(m_ListenHandler != NULL)
		delete m_ListenHandler;
}

bool IAppInterface::Listen(int32_t port, const char *ip/*=NULL*/, uint32_t back_log/*=128*/)
{
	int32_t fd = Socket::CreateListenSocket(port, ip, false);
	if(fd == -1)
	{
		LOG_ERROR(logger, "create listen socket failed. port="<<port<<" errno="<<errno<<"("<<strerror(errno)<<")");
		return false;
	}
	if(Socket::Listen(fd, back_log) == false)
	{
		LOG_ERROR(logger, "listen failed. fd="<<fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
		Socket::Close(fd);
		return false;
	}

	IEventServer *event_server = GetEventServer();
	IEventHandler *event_handler = GetListenHander();
	assert(event_server != NULL);
	assert(event_handler != NULL);
	if(!event_server->AddEvent(fd, ET_PER_RD, event_handler, -1))
	{
		LOG_ERROR(logger, "add perist read event to event_server failed. fd="<<fd);
		Socket::Close(fd);
		return false;
	}
	return true;
}

bool IAppInterface::AcceptNewConnect(int32_t fd)
{
	const char *peer_ip = "_unknow ip_";
	int16_t peer_port = -1;
	struct sockaddr_in peer_addr;
	int socket_len = sizeof(peer_addr);

	if(getpeername(fd, (struct sockaddr*)&peer_addr, (socklen_t*)&socket_len) == 0)
	{
		peer_ip = inet_ntoa(peer_addr.sin_addr);
		peer_port = ntohs(peer_addr.sin_port);
	}

	LOG_DEBUG(logger, "accept new connect. fd="<<fd<<" peer_ip="<<peer_ip<<" peer_port="<<peer_port);
	int32_t time_out = GetIdleTimeout();
	IEventServer *event_server = GetEventServer();
	IEventHandler* event_handler = GetTransHandler();
	assert(event_handler != NULL);
	assert(event_server != NULL);
	if(!event_server->AddEvent(fd, ET_PER_RD, event_handler, time_out))
	{
		LOG_ERROR(logger, "add persist read event to event_server failed when send protocol. fd="<<fd);
		return false;
	}
	return true;
}

bool IAppInterface::SendProtocol(int32_t fd, ProtocolContext *context, int32_t send_timeout/*=-1*/)
{
	if(fd<0 || context==NULL)
		return false;
	context->time_out = send_timeout;
	if(context->time_out > 0)
	{
		uint64_t now;
		GetCurTime(now);
		context->expire_time = now+context->time_out;
	}

	SendMap::iterator it = m_SendMap.find(fd);
	if(it == m_SendMap.end())
	{
		ProtocolList temp_list;
		std::pair<SendMap::iterator, bool> result = m_SendMap.insert(std::make_pair(fd, temp_list));
		if(result.second == false)
		{
			LOG_ERROR(logger, "add protocol to list failed. fd="<<fd<<" context="<<context);
			return false;
		}
		it = result.first;
	}

	ProtocolList *protocol_list = &it->second;
	/*ProtocolList::iterator list_it = protocol_list->begin();
	for(; list_it!=protocol_list->end(); ++list_it)    //按超时时间点从小到大排列
	{
		ProtocolContext *temp_context = *list_it;
		if(context->expire_time < temp_context->expire_time)
			break;
	}
	protocol_list->insert(list_it, context);
	*/
	protocol_list->push_back(context);    //直接添加到队列尾

	//添加可写事件监控
	int32_t time_out = GetIdleTimeout();
	IEventServer *event_server = GetEventServer();
	IEventHandler* event_handler = GetTransHandler();
	assert(event_server != NULL);
	assert(event_handler != NULL);
	if(!event_server->AddEvent(fd, ET_WRITE, event_handler, time_out))
	{
		LOG_ERROR(logger, "add write event to event_server failed when send protocol. fd="<<fd<<" context="<<context);
		return false;
	}
	return true;
}

ProtocolContext* IAppInterface::GetSendProtocol(int32_t fd)
{
	SendMap::iterator it = m_SendMap.find(fd);
	if(it == m_SendMap.end())
		return NULL;
	ProtocolList *protocol_list = &it->second;
	ProtocolContext *context = NULL;
	if(!protocol_list->empty())
	{
		context = protocol_list->front();
		protocol_list->pop_front();
	}

	if(protocol_list->empty())
		m_SendMap.erase(it);

	return context;
}


//创建协议上下文
ProtocolContext* IAppInterface::NewProtocolContext()
{
	IMemory *memory = GetMemory();
	void *buf = memory->Alloc(sizeof(ByteBuffer));
	void *mem = memory->Alloc(sizeof(ProtocolContext));
	assert(mem!=NULL && buf!=NULL);

	ByteBuffer *bytebuffer = new(buf) ByteBuffer(1024, memory);
	ProtocolContext *context = new(mem) ProtocolContext;
	context->bytebuffer = bytebuffer;
	return context;
}

//删除协议上下文
void IAppInterface::DeleteProtocolContext(ProtocolContext *context)
{
	//释放protocol实例
	IProtocolFactory *factory = GetProtocolFactory();
	factory->DeleteProtocol(context->protocol_type, context->protocol);

	IMemory *memory = GetMemory();
	assert(context!=NULL && context->bytebuffer!=NULL);
	ByteBuffer *bytebuffer = context->bytebuffer;
	bytebuffer->~ByteBuffer();
	memory->Free((void*)bytebuffer, sizeof(ByteBuffer));
	memory->Free((void*)context, sizeof(ProtocolContext));
}

//获取EventServer的实例
IEventServer* IAppInterface::GetEventServer()
{
	if(m_EventServer == NULL)
		m_EventServer = new EventServerEpoll(10000);
	return m_EventServer;
}

//获取ProtocolFactory的实例
IProtocolFactory* IAppInterface::GetProtocolFactory()
{
	if(m_ProtocolFactory)
		m_ProtocolFactory = new KVDataProtocolFactory(GetMemory());
	return m_ProtocolFactory;
}

//获取传输handler
IEventHandler* IAppInterface::GetTransHandler()
{
	if(m_TransHandler == NULL)
		m_TransHandler = new TransHandler(this);
	return m_TransHandler;
}

//服务监听handler
IEventHandler* IAppInterface::GetListenHander()
{
	if(m_ListenHandler == NULL)
		m_ListenHandler = new ListenHandler(this);
	return m_ListenHandler;
}

IMemory* IAppInterface::GetMemory()
{
	return &m_SysMemory;
}

}//namespace
