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

IAppInterface::~IAppInterface()
{
	if(m_SendMap != NULL)
		delete m_SendMap;
}

void IAppInterface::OnCreateAppInstance()
{
	m_EventServer     = CreateEventServer();
	assert(m_EventServer != NULL);
	m_ProtocolFactory = CreateProtocolFactory();
	assert(m_ProtocolFactory != NULL);
}

void IAppInterface::OnDestroyAppInstance()
{
	DestroyEventServer(m_EventServer);
	DestroyProtocolFactory(m_ProtocolFactory);
}

bool IAppInterface::SendProtocol(int32_t fd, ProtocolContext *context)
{
	if(fd<0 || context==NULL)
		return false;
	if(m_SendMap == NULL)
		m_SendMap = new SendMap;

	SendMap::iterator it = m_SendMap->find(fd);
	if(it == m_SendMap->end())
	{
		ProtocolList temp_list;
		std::pair<SendMap::iterator, bool> result = m_SendMap->insert(std::make_pair(fd, temp_list));
		if(result.second == false)
			return false;
		it = result.first;
	}

	ProtocolList *protocol_list = &it->second;
	ProtocolList::iterator list_it = protocol_list->begin();
	for(; list_it!=protocol_list->end(); ++list_it)    //按超时时间点从小到大排列
	{
		ProtocolContext *temp_context = *list_it;
		if(context->expire_time < temp_context->expire_time)
			break;
	}
	protocol_list->insert(list_it, context);

	//TODO
	//add to event server

	return true;
}

ProtocolContext* IAppInterface::GetSendProtocol(int32_t fd)
{
	SendMap::iterator it = m_SendMap->find(fd);
	if(it == m_SendMap->end())
		return NULL;
	ProtocolList *protocol_list = &it->second;
	ProtocolContext *context = NULL;
	if(!protocol_list->empty())
	{
		context = protocol_list->front();
		protocol_list->pop_front();
	}

	if(protocol_list->empty())
		m_SendMap->erase(it);

	return context;
}
}//namespace
