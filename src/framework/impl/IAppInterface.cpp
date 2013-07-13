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
	,m_ListenHandler(NULL)
	,m_MessageHandler(NULL)
	,m_RecvFd(-1)
	,m_WriteFd(-1)
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
	if(m_MessageHandler != NULL)
		delete m_MessageHandler;

	if(m_RecvFd > 0)
		close(m_RecvFd);
	if(m_WriteFd > 0)
		close(m_RecvFd);
}

//获取EventServer的实例
IEventServer* IAppInterface::GetEventServer()
{
	if(m_EventServer == NULL)
	{
		int32_t max_connexctions = GetMaxConnections();
		m_EventServer = new EventServerEpoll(max_connexctions);
	}
	return m_EventServer;
}

//获取ProtocolFactory的实例
IProtocolFactory* IAppInterface::GetProtocolFactory()
{
	if(m_ProtocolFactory == NULL)
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
IEventHandler* IAppInterface::GetMessageHandler()
{
	if(m_MessageHandler == NULL)
		m_MessageHandler = new MessageHandler(this);
	return m_MessageHandler;
}

IMemory* IAppInterface::GetMemory()
{
	return &m_SysMemory;
}

bool IAppInterface::Listen(int32_t port, const char *ip/*=NULL*/, uint32_t back_log/*=128*/)
{
	int32_t fd = Socket::CreateListenSocket(port, ip, false);
	if(fd == -1)
	{
		LOG_ERROR(logger, "create listen socket failed. port="<<port<<", errno="<<errno<<"("<<strerror(errno)<<").");
		return false;
	}
	if(Socket::Listen(fd, back_log) == false)
	{
		LOG_ERROR(logger, "listen failed. fd="<<fd<<", errno="<<errno<<"("<<strerror(errno)<<").");
		Socket::Close(fd);
		return false;
	}

	IEventServer *event_server = GetEventServer();
	IEventHandler *event_handler = GetListenHander();
	assert(event_server != NULL);
	assert(event_handler != NULL);
	if(!event_server->SetEvent(fd, ET_PER_RD, event_handler, -1))
	{
		LOG_ERROR(logger, "add persist read event to event_server failed. fd="<<fd);
		Socket::Close(fd);
		return false;
	}
	return true;
}

void IAppInterface::OnListenError(int32_t fd)
{
	LOG_ERROR(logger, "close listen socket. fd="<<fd);
	Socket::Close(fd);
}

bool IAppInterface::AcceptNewConnect(int32_t fd)
{
	const char *peer_ip = "_unknow ip_";
	uint16_t peer_port = 0;
	struct sockaddr_in peer_addr;
	int socket_len = sizeof(peer_addr);

	if(getpeername(fd, (struct sockaddr*)&peer_addr, (socklen_t*)&socket_len) == 0)
	{
		peer_ip = inet_ntoa(peer_addr.sin_addr);
		peer_port = ntohs(peer_addr.sin_port);
	}

	LOG_DEBUG(logger, "accept new connect. new_fd="<<fd<<", peer_ip="<<peer_ip<<", peer_port="<<peer_port);
	int32_t timeout_ms = GetSocketIdleTimeout();
	IEventServer *event_server = GetEventServer();
	IEventHandler* event_handler = GetTransHandler();
	assert(event_handler != NULL);
	assert(event_server != NULL);
	if(!event_server->SetEvent(fd, ET_PER_RD, event_handler, timeout_ms))
	{
		LOG_ERROR(logger, "add persist read event to event_server failed when send protocol. fd="<<fd);
		return false;
	}
	return true;
}

//创建协议上下文
ProtocolContext* IAppInterface::NewProtocolContext()
{
	IMemory *memory = GetMemory();
	void *mem = memory->Alloc(sizeof(ProtocolContext));
	assert(mem!=NULL);
	ProtocolContext *context = new(mem) ProtocolContext(1024, memory);
	return context;
}

//删除协议上下文
void IAppInterface::DeleteProtocolContext(ProtocolContext *context)
{
	//释放protocol实例
	IProtocolFactory *factory = GetProtocolFactory();
	if(context->protocol != NULL)
		factory->DeleteProtocol(context->protocol_type, context->protocol);

	IMemory *memory = GetMemory();
	assert(context!=NULL);
	context->~ProtocolContext();
	memory->Free((void*)context, sizeof(ProtocolContext));
}

bool IAppInterface::SendProtocol(int32_t fd, ProtocolContext *context, int32_t send_timeout_ms/*=-1*/)
{
	if(fd<0 || context==NULL)
		return false;
	context->timeout_ms = send_timeout_ms;
	if(context->timeout_ms > 0)
	{
		uint64_t now;
		GetCurTime(now);
		context->expire_time = now+context->timeout_ms;
	}

	SendMap::iterator it = m_SendMap.find(fd);
	if(it == m_SendMap.end())
	{
		ProtocolList temp_list;
		std::pair<SendMap::iterator, bool> result = m_SendMap.insert(std::make_pair(fd, temp_list));
		if(result.second == false)
		{
			LOG_ERROR(logger, "add protocol to list failed. fd="<<fd<<", context="<<context);
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
	int32_t timeout_ms = GetSocketIdleTimeout();
	IEventServer *event_server = GetEventServer();
	IEventHandler* event_handler = GetTransHandler();
	assert(event_server != NULL);
	assert(event_handler != NULL);
	if(!event_server->SetEvent(fd, ET_WRITE, event_handler, timeout_ms))
	{
		LOG_ERROR(logger, "add write event to event_server failed when send protocol. fd="<<fd<<", context="<<context);
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

bool IAppInterface::NotifySocketFinish(int32_t fd)
{
	LOG_DEBUG(logger, "notify socket finish. fd="<<fd);
	//从EventServer中删除所有关于fd的事件
	IEventServer *event_server = GetEventServer();
	assert(event_server != NULL);
	if(event_server->DeleteEvent(fd, ET_RDWT) == false)
	{
		LOG_ERROR(logger, "delete all event from event server failed when notify socket finish. fd="<<fd);
		return false;
	}

	//相应调用事件相应函数
	IEventHandler *trans_handler = GetTransHandler();
	assert(trans_handler != NULL);
	trans_handler->OnEventError(fd, 0, ECODE_ACTIVE_CLOSE);
	return true;
}

bool IAppInterface::ListenMessage()
{
	//创建sock pair
	int32_t fd[2];
	int32_t ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
	if(ret == -1)
	{
		LOG_ERROR(logger, "create socket pair failed. errno="<<errno<<"("<<strerror(errno)<<")");
		return false;
	}

	if(Socket::SetNoBlock(fd[0]) == false)
	{
		LOG_ERROR(logger, "set message fd noblock failed. fd="<<fd[0]);
		close(fd[0]);
		close(fd[1]);
	}
	LOG_DEBUG(logger, "set message fd noblock succ. fd="<<fd[0]);

	IEventServer *event_server = GetEventServer();
	IEventHandler *event_handler = GetMessageHandler();
	assert(event_server != NULL);
	assert(event_handler != NULL);
	if(!event_server->SetEvent(fd[0], ET_PER_RD, event_handler, -1))
	{
		LOG_ERROR(logger, "add persist read event to event_server failed. fd="<<fd[0]);
		close(fd[0]);
		close(fd[1]);
		return false;
	}

	LOG_DEBUG(logger, "add message listen to event server succ. read_fd="<<fd[0]<<" write_fd="<<fd[1]);
	m_RecvFd = fd[0];
	m_WriteFd = fd[1];

	return true;
}

bool IAppInterface::SendMessage(int32_t msg)
{
	if(write(m_WriteFd, &msg, sizeof(msg)) == -1)
	{
		LOG_ERROR(logger, "send msg="<<msg<<"failed. write_fd="<<m_WriteFd);
		return false;
	}
	LOG_DEBUG(logger, "send msg="<<msg<<"succ. write_fd="<<m_WriteFd);
	return true;
}

//处理消息通知
bool IAppInterface::OnRecvMessage(int32_t msg)
{
	LOG_DEBUG(logger, "recv message. msg="<<msg);
	//默认收到的是fd,添加到EventSever
	if(msg > 0)
		return AcceptNewConnect(msg);
	else
		return true;
}

}//namespace
