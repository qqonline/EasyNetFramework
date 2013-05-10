/*
 * IProtocolFactory.cpp
 *
 *  Created on: May 6, 2013
 *      Author: LiuYongJin
 */

#include <assert.h>
#include <string.h>

#include "IProtocolFactory.h"


namespace easynet
{

//最小的接收缓冲区大小
#define MIN_BUFFER_SIZE 128

IProtocolFactory::~IProtocolFactory()
{
	if(m_MemPool != NULL)
		delete m_MemPool;
}

ProtocolContext* IProtocolFactory::NewRecvContext(uint64_t now_time)
{
	if(m_MemPool == NULL)
		m_MemPool = new MemPool;

	ProtocolContext *context = (ProtocolContext *)m_MemPool->Alloc(sizeof(ProtocolContext));
	if(context == NULL)
		return NULL;
	context->Init(this);
	context->time_out = m_RecvTimeout;    //接收超时时间
	if(context->time_out > 0)
		context->expire_time = now_time+context->time_out;

	InitRecvInfo((ProtocolInfo*)context);
	assert((context->data_type==DTYPE_INVALID&&context->data_header_size>0)
			|| (context->data_type==DTYPE_TEXT&&context->body_size>0)
			|| (context->data_type==DTYPE_BIN&&context->header_size>0));

	//分配缓冲区
	if(context->data_type==DTYPE_INVALID)
		context->buffer_size = context->data_header_size;
	else if(context->data_type==DTYPE_TEXT)
		context->buffer_size = context->body_size;
	else if(context->data_type==DTYPE_BIN)
		context->buffer_size = context->header_size;

	if(context->buffer_size < MIN_BUFFER_SIZE)
		context->buffer_size = MIN_BUFFER_SIZE;
	context->buffer = (char*)m_MemPool->Alloc(context->buffer_size);
	if(context->buffer == NULL)
	{
		m_MemPool->Free((void*)context, sizeof(ProtocolContext));
		context = NULL;
	}
	return context;
}

ProtocolContext* IProtocolFactory::NewSendContext(DataType data_type, uint32_t max_data_size, uint32_t protocol_type)
{
	assert(data_type!=DTYPE_INVALID && max_data_size>0);

	if(m_MemPool == NULL)
		m_MemPool = new MemPool;

	ProtocolContext *context = (ProtocolContext *)m_MemPool->Alloc(sizeof(ProtocolContext));
	if(context == NULL)
		return NULL;
	context->Init(this);

	context->data_type = data_type;
	context->protocol_type = protocol_type;
	context->buffer_size = max_data_size;
	context->buffer = m_MemPool->Alloc(context->buffer_size);
	if(context->buffer == NULL)
	{
		m_MemPool->Free((void*)context, sizeof(ProtocolContext));
		context = NULL;
	}
	else if(context->data_type == DTYPE_BIN)
	{
		context->protocol = NewProtocol(context->protocol_type, context->buffer, context->buffer_size);
		if(context->protocol == NULL)
		{
			m_MemPool->Free(context->buffer, context->buffer_size);
			m_MemPool->Free((void*)context, sizeof(ProtocolContext));
		}
		context = NULL;
	}

	return context;
}

void IProtocolFactory::DeleteContext(ProtocolContext *context)
{
	if(context == NULL)
		return ;
	if(context->protocol != NULL)
		DeleteProtocol(context->protocol, context->protocol_type);
	if(context->buffer != NULL)
		m_MemPool->Free((void*)context->buffer, context->buffer_size);
	m_MemPool->Free((void*)context, sizeof(ProtocolContext));
}

}//namespace


