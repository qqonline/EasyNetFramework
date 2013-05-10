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

bool KVDataProtocolFactory::InitRecvInfo(ProtocolInfo *pro_info)
{
	pro_info->data_type = DTYPE_BIN;
	pro_info->header_size = 8;
	return true;
}

DecodeResult KVDataProtocolFactory::DecodeDataType(ProtocolContext *context)
{
	assert(0);
	return DECODE_SUCC;
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
		uint32_t size0, size1;
		kv_data->DetachBuffer(buffer, size0, size1);

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
		kv_data->AttachBuffer(buffer, context->buffer_size, context->cur_data_size);
	}
	return DECODE_SUCC;
}

DecodeResult KVDataProtocolFactory::DecodeBinBody(ProtocolContext *context)
{
	KVData *kv_data = (KVData*)context->protocol;
	return kv_data->UnPack()?DECODE_SUCC:DECODE_ERROR;
}

DecodeResult KVDataProtocolFactory::DecodeTextBody(ProtocolContext *context)
{
	assert(0);
	return DECODE_SUCC;
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

bool KVDataProtocolFactory::EncodeProtocol(void *protocol, int32_t protocol_type, char *buffer, uint32_t buffer_size)
{
	//magic_num:"KVPF"
	buffer[0] = 'K';
	buffer[1] = 'V';
	buffer[2] = 'P';
	buffer[3] = 'F';
	buffer += 4;

	//body_size
	KVData *kv_data = (KVData*)protocol->protocol;
	uint32_t body_size = kv_data->Size();
	*((uint32_t*)buffer) = htonl(body_size);

	LOG_DEBUG(logger, "encode protocl succ. header_size=4 body_size="<<body_size);
	return true;
}

}//namespace
