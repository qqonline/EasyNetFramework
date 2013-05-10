/*
 * IAppInterface.cpp
 *
 *  Created on: 2013-5-9
 *      Author: LiuYongJin
 */
#include <assert.h>

#include "IAppInterface.h"

namespace easynet
{
IMPL_LOGGER(IAppInterface, logger);

#define GetCurTime(now) do{                  \
	struct timeval tv;                        \
	gettimeofday(&tv, NULL);                  \
	now = tv.tv_sec*1000+tv.tv_usec/1000;     \
}while(0)

bool IAppInterface::SendProtocol(int32_t fd, ProtocolContext *context)
{
	if(fd<0 || context==NULL)
		return false;
	if(context->time_out >= 0)
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
	IEventServer *event_server = GetEventServer();
	assert(event_server != NULL);
	if(!event_server->AddEvent(fd, ET_WRITE, &m_TransHandler, context->time_out))
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
}//namespace
