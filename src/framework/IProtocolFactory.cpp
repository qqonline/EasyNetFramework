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

IProtocolFactory::IProtocolFactory(IMemory *memory)
	:m_Memory(memory)
{
	assert(m_Memory != NULL);
}

ProtocolContext* IProtocolFactory::NewRecvContext()
{
	void *mem_block = m_Memory->Alloc(sizeof(ProtocolContext));
	if(mem_block == NULL)
		return NULL;
	ProtocolContext *context = new(mem_block) ProtocolContext(this);
	InitDecodeParameter(context);

	//分配缓冲区
	if(context->data_type == DTYPE_INVALID)    //支持两种格式
		context->buffer_size = context->header_size;
	if(context->data_type == DTYPE_TEXT)
		context->buffer_size = context->body_size;
	else if(context->data_type == DTYPE_BIN)
		context->buffer_size = context->header_size;
	else
	{
		m_Memory->Free((void*)context, sizeof(ProtocolContext));
		return NULL;
	}

	if(context->buffer_size < MIN_BUFFER_SIZE)
		context->buffer_size = MIN_BUFFER_SIZE;
	context->buffer = (char*)m_Memory->Alloc(context->buffer_size);
	if(context->buffer == NULL)
	{
		m_Memory->Free((void*)context, sizeof(ProtocolContext));
		return NULL;
	}

	return context;
}

ProtocolContext* IProtocolFactory::NewSendContextText(uint32_t max_size)
{
	assert(max_size > 0);
	void *mem_block = m_Memory->Alloc(sizeof(ProtocolContext));
	if(mem_block == NULL)
		return NULL;
	ProtocolContext *context = new(mem_block) ProtocolContext(this);

	context->data_type     = DTYPE_TEXT;
	context->protocol_type = -1;
	context->buffer_size   = max_size;
	context->buffer        = (char*)m_Memory->Alloc(context->buffer_size);
	if(context->buffer == NULL)
	{
		m_Memory->Free((void*)context, sizeof(ProtocolContext));
		return NULL;
	}

	return context;
}

ProtocolContext* IProtocolFactory::NewSendContextBin(uint32_t protocol_type, uint32_t max_size)
{
	void *mem_block = m_Memory->Alloc(sizeof(ProtocolContext));
	if(mem_block == NULL)
		return NULL;
	ProtocolContext *context = new(mem_block) ProtocolContext(this);

	context->data_type     = DTYPE_BIN;
	context->header_size   = GetBinHeaderSize();
	context->protocol_type = protocol_type;
	context->buffer_size   = context->header_size+max_size;
	context->buffer        = (char*)m_Memory->Alloc(context->buffer_size);
	if(context->buffer == NULL)
	{
		m_Memory->Free((void*)context, sizeof(ProtocolContext));
		return NULL;
	}

	//二进制协议,创建具体的协议实例
	context->protocol = NewProtocol(context->protocol_type, context->buffer+context->header_size, context->body_size);
	if(context->protocol == NULL)
	{
		m_Memory->Free(context->buffer, context->buffer_size);
		m_Memory->Free((void*)context, sizeof(ProtocolContext));
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
		m_Memory->Free((void*)context->buffer, context->buffer_size);
	m_Memory->Free((void*)context, sizeof(ProtocolContext));
}

bool IProtocolFactory::ReAllocBuffer(ProtocolContext *context, uint32_t new_size)
{
	if(context->buffer_size >= new_size)
		return true;

	void *new_buffer = m_Memory->ReAlloc(context->buffer, context->buffer_size, new_size);
	if(new_buffer == NULL)
		return false;
	//设置新的buffer
	context->buffer = (char*)new_buffer;
	context->buffer_size = new_size;
	return true;
}

}//namespace


