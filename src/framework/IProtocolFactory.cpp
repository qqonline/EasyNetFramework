/*
 * IProtocolFactory.cpp
 *
 *  Created on: May 6, 2013
 *      Author: LiuYongJin
 */

#include <assert.h>
#include <string.h>
#include <new>

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

ProtocolContext* IProtocolFactory::NewRecvContext()
{
	if(m_MemPool == NULL)
		m_MemPool = new MemPool;

	void *mem_block = m_MemPool->Alloc(sizeof(ProtocolContext));
	if(mem_block == NULL)
		return NULL;
	ProtocolContext *context = new(mem_block) ProtocolContext(this);
	InitRecvDefine((ProtocolDefine*)context);
	assert((context->data_type==DTYPE_ALL&&context->data_header_size>0)
			|| (context->data_type==DTYPE_TEXT&&context->body_size>0)
			|| (context->data_type==DTYPE_BIN&&context->header_size>0));

	//分配缓冲区
	if(context->data_type == DTYPE_ALL)
		context->buffer_size = context->data_header_size;
	else if(context->data_type == DTYPE_TEXT)
		context->buffer_size = context->body_size;
	else if(context->data_type == DTYPE_BIN)
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

ProtocolContext* IProtocolFactory::NewSendContext(DataType data_type, uint32_t max_data_size, int32_t protocol_type)
{
	if(m_MemPool == NULL)
		m_MemPool = new MemPool;

	assert(max_data_size>0);
	void *mem_block = m_MemPool->Alloc(sizeof(ProtocolContext));
	if(mem_block == NULL)
		return NULL;
	ProtocolContext *context = new(mem_block) ProtocolContext(this);

	context->data_type     = data_type;
	context->protocol_type = protocol_type;
	context->buffer_size   = max_data_size;
	context->buffer        = (char*)m_MemPool->Alloc(context->buffer_size);
	if(context->buffer == NULL)
	{
		m_MemPool->Free((void*)context, sizeof(ProtocolContext));
		return NULL;
	}

	//二进制协议,创建具体的协议实例
	if(context->data_type == DTYPE_BIN)
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

bool IProtocolFactory::ReAllocBuffer(ProtocolContext *context, uint32_t new_size)
{
	if(context->buffer_size >= new_size)
		return true;

	//分配新的buffer
	void *new_buffer = m_MemPool->Alloc(new_size);
	if(new_buffer == NULL)
		return false;
	//复制数据到新的buffer
	memcpy(new_buffer, context->buffer, context->cur_data_size);
	//回收旧的buffer
	m_MemPool->Free(context->buffer, context->buffer_size);
	//设置新的buffer
	context->buffer = (char*)new_buffer;
	context->buffer_size = new_size;

	return true;
}

}//namespace


