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

#define MIN_BUFFER_SIZE 128

IProtocolFactory::IProtocolFactory():m_MemPool(NULL)
{
}

IProtocolFactory::~IProtocolFactory()
{
	if(m_MemPool != NULL)
		delete m_MemPool;
}

RecvContext* IProtocolFactory::NewRecvContext()
{
	if(m_MemPool == NULL)
		m_MemPool = new MemPool;

	RecvContext *context = (RecvContext *)m_MemPool->Alloc(sizeof(RecvContext));
	if(context != NULL)
	{
		context->Init();
		InitProtocolInfo((ProtocolInfo*)context);

		assert((context->data_type==DTYPE_INVALID&&context->data_header_size>0)
				|| (context->data_type==DTYPE_TEXT&&context->body_size>0)
				|| (context->data_type==DTYPE_BIN&&context->header_size>0));

		if(context->data_type==DTYPE_INVALID)
			context->buffer_size = context->data_header_size;
		else if(context->data_type==DTYPE_TEXT)
			context->buffer_size = context->body_size;
		else if(context->data_type==DTYPE_BIN)
			context->buffer_size = context->header_size;

		if(context->buffer_size < MIN_BUFFER_SIZE)
			context->buffer_size = MIN_BUFFER_SIZE;
		context->buffer = (char*)m_MemPool->Alloc(context->buffer_size);
	}
	return context;
}

void IProtocolFactory::DeleteRecvContext(RecvContext *context)
{
	assert(context != NULL);
	if(context->buffer != NULL)
		m_MemPool->Free((void*)context->buffer, context->buffer_size);
	m_MemPool->Free((void*)context, sizeof(RecvContext));
}

}//namespace


