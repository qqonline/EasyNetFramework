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

void ProtocolContext::Destroy()
{
	m_Factory->DeleteContext(this);
}

ProtocolContext::ProtocolContext(IProtocolFactory *factory, IMemory *mem)
		:type(DTYPE_UNKNOW)
		,expect_size(0)
		,bytebuffer(NULL)
		,time_out(-1)
		,expire_time(-1)
		,fd(-1)
		,m_Memory(mem)
		,m_Factory(factory)
{
	assert(mem != NULL);
	bytebuffer = ByteBuffer::Create(1024, mem);
}

ProtocolContext::~ProtocolContext()
{
	bytebuffer->Destroy();
}

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
	ProtocolContext *context = new(mem_block) ProtocolContext(this, m_Memory);
	InitDecodeParameter(context->type,context->expect_size);
	if(context->expect_size > context->bytebuffer->m_Capacity)
		context->bytebuffer->Enlarge(context->expect_size);
	return context;
}

ProtocolContext* IProtocolFactory::NewSendContextText(uint32_t max_size)
{
	assert(max_size > 0);
	void *mem_block = m_Memory->Alloc(sizeof(ProtocolContext));
	if(mem_block == NULL)
		return NULL;
	ProtocolContext *context = new(mem_block) ProtocolContext(this, m_Memory);

	context->type = DTYPE_TEXT;
	if(max_size > context->bytebuffer->m_Capacity)
		context->bytebuffer->Enlarge(max_size);
	return context;
}

ProtocolContext* IProtocolFactory::NewSendContextBin(uint32_t protocol_type, uint32_t max_size)
{
	void *mem_block = m_Memory->Alloc(sizeof(ProtocolContext));
	if(mem_block == NULL)
		return NULL;
	ProtocolContext *context = new(mem_block) ProtocolContext(this, m_Memory);

	context->type = DTYPE_BIN;
	uint32_t header_size = GetBinHeaderSize();
	if(header_size > context->bytebuffer->m_Capacity)
		context->bytebuffer->Enlarge(header_size);
	context->bytebuffer->m_Size = header_size;

	context->protocol_type = protocol_type;
	//二进制协议,创建具体的协议实例
	context->protocol = NewProtocol(context->protocol_type, context->bytebuffer);
	assert(context->protocol != NULL);

	return context;
}

void IProtocolFactory::DeleteContext(ProtocolContext *context)
{
	if(context == NULL)
		return ;
	if(context->protocol != NULL)
		DeleteProtocol(context->protocol_type, context->protocol);
	context->~ProtocolContext();
	m_Memory->Free((void*)context, sizeof(ProtocolContext));
}

}//namespace


