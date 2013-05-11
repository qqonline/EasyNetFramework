/*
 * KVDataProtocolFactory.cpp
 *
 *  Created on: 2013-5-10
 *      Author: LiuYongJin
 */
#include <new>
#include <netinet/in.h>

#include "KVDataProtocolFactory.h"
#include "KVData.h"

namespace easynet
{
IMPL_LOGGER(KVDataProtocolFactory, logger);

#define KVDATA_HEADER_SIZE 8    //magic+body_len

void KVDataProtocolFactory::InitRecvDefine(ProtocolDefine *protocol_def)
{
	protocol_def->data_type = DTYPE_BIN;
	protocol_def->header_size = KVDATA_HEADER_SIZE;
	return ;
}

DecodeResult KVDataProtocolFactory::DecodeBinHeader(ProtocolContext *context)
{
	char *buffer = context->buffer;
	//magic_num: "KVPF"
	if(buffer[0]!='K' || buffer[1]!='V' ||buffer[2]!='P' || buffer[3]!='F')
	{
		LOG_ERROR(logger, "magic_num invalid. my_magic_num='KVPF' recv_magic_num='"<<buffer[0]<<buffer[1]<<buffer[2]<<buffer[3]<<"'");
		return DECODE_ERROR;
	}
	uint32_t body_size = *(uint32_t*)(buffer+4);
	context->body_size = ntohl(body_size);

	//分配的空间不够,重新分配
	if(body_size+KVDATA_HEADER_SIZE > context->buffer_size)
	{
		//先剥离旧的buffer
		KVData *kv_data = (KVData*)context->protocol;
		void *buffer;
		uint32_t buffer_size, data_size;
		kv_data->DetachBuffer(buffer, buffer_size, data_size);

		//重新分配内存
		if(!ReAllocBuffer(context, body_size+KVDATA_HEADER_SIZE))
		{
			LOG_ERROR(logger, "realloc buffer failed. old_size="<<context->buffer_size
						<<" new_size="<<body_size+KVDATA_HEADER_SIZE);
			return DECODE_ERROR;
		}

		//重新设置buffer
		buffer = context->buffer;
		buffer = (void*)((char*)buffer+KVDATA_HEADER_SIZE);
		buffer_size = context->buffer_size-KVDATA_HEADER_SIZE;
		data_size = context->cur_data_size-KVDATA_HEADER_SIZE;
		kv_data->AttachBuffer(buffer, buffer_size, data_size, false);
	}

	return DECODE_SUCC;
}

DecodeResult KVDataProtocolFactory::DecodeBinBody(ProtocolContext *context)
{
	KVData *kv_data = (KVData*)context->protocol;
	return kv_data->UnPack()?DECODE_SUCC:DECODE_ERROR;
}

void* KVDataProtocolFactory::NewProtocol(int32_t protocol_type, char *buffer, uint32_t buffer_size)
{
	void *temp = m_MemPool->Alloc(sizeof(KVData));
	if(temp == NULL)
		return NULL;

	//前面KVDATA_HEADER_SIZE字节是协议头的数据
	KVData* kv_data = new(temp) KVData(buffer+KVDATA_HEADER_SIZE, buffer_size-KVDATA_HEADER_SIZE);    //使用外部提供的buffer
	return kv_data;
}

void KVDataProtocolFactory::DeleteProtocol(void *protocol, int32_t protocol_type)
{
	m_MemPool->Free(protocol,sizeof(KVData));
}

//对协议进行编码
bool KVDataProtocolFactory::EncodeProtocol(ProtocolContext *send_context)
{
	return EncodeProtocol(send_context->protocol, send_context->protocol_type, send_context->buffer, send_context->buffer_size);
}

bool KVDataProtocolFactory::EncodeProtocol(void *protocol, int32_t protocol_type, char *buffer, uint32_t buffer_size)
{
	assert(protocol!=NULL);
	//magic_num:"KVPF"
	buffer[0] = 'K';
	buffer[1] = 'V';
	buffer[2] = 'P';
	buffer[3] = 'F';
	buffer += 4;

	//body_size
	KVData *kv_data = (KVData*)protocol;
	uint32_t body_size = kv_data->Size();
	*((uint32_t*)buffer) = htonl(body_size);

	LOG_DEBUG(logger, "encode protocol succ. header_size=8 body_size="<<body_size);
	return true;
}

}//namespace
