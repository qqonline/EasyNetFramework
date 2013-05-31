/*
 * KVDataProtocolFactory.cpp
 *
 *  Created on: 2013-5-10
 *      Author: LiuYongJin
 */
#include <new>
#include <netinet/in.h>
#include <assert.h>

#include "KVDataProtocolFactory.h"
#include "KVData.h"

namespace easynet
{
IMPL_LOGGER(KVDataProtocolFactory, logger);

#define KVDATA_HEADER_SIZE 8    //magic+body_len

//头部大小
uint32_t KVDataProtocolFactory::HeaderSize()
{
	return KVDATA_HEADER_SIZE;
}

//从buffer中解码头部,并获取协议体数据.成功返回true,失败返回false.
DecodeResult KVDataProtocolFactory::DecodeHeader(const char *buffer, DataType &type, uint32_t &body_size)
{
	//magic_num: "KVPF"
	if(buffer[0]!='K' || buffer[1]!='V' ||buffer[2]!='P' || buffer[3]!='F')
	{
		LOG_ERROR(logger, "decode KVDataProtocolFactory header failed. my_magic_num='KVPF' recv_magic_num='"<<buffer[0]<<buffer[1]<<buffer[2]<<buffer[3]<<"'");
		return DECODE_ERROR;
	}
	type = DTYPE_BIN;
	body_size = *(uint32_t*)(buffer+4);
	body_size = ntohl(body_size);
	return DECODE_SUCC;
}

//将头部数据编码写入到buffer.body_size为协议体大小
void KVDataProtocolFactory::EncodeHeader(char *buffer, uint32_t body_size)
{
	//magic_num:"KVPF"
	buffer[0] = 'K';
	buffer[1] = 'V';
	buffer[2] = 'P';
	buffer[3] = 'F';
	buffer += 4;

	*((uint32_t*)buffer) = htonl(body_size);
}

DecodeResult KVDataProtocolFactory::DecodeBinBody(ProtocolContext *context)
{
	if(context->bytebuffer->m_Size < context->header_size+context->body_size)
		return DECODE_DATA;

	void *mem = NULL;
	if(m_Memory != NULL)
	{
		mem = m_Memory->Alloc(sizeof(KVData));
	}
	else
	{
		SystemMemory sys_memory;
		mem = sys_memory.Alloc(sizeof(KVData));
	}
	assert(mem != NULL);
	KVData *kvdata = new(mem) KVData;

	ByteBuffer *bytebuffer = context->bytebuffer;
	char *buffer = bytebuffer->m_Buffer+KVDATA_HEADER_SIZE;
	kvdata->AttachReadBuffer(buffer, context->body_size, true);
	if(kvdata->UnPack() == false)
	{
		if(m_Memory != NULL)
		{
			m_Memory->Free((void*)kvdata, sizeof(KVData));
		}
		else
		{
			SystemMemory sys_memory;
			sys_memory.Free((void*)kvdata, sizeof(KVData));
		}
		return DECODE_ERROR;
	}
	context->protocol = (void*)kvdata;
	return DECODE_SUCC;
}

DecodeResult KVDataProtocolFactory::DecodeTextBody(ProtocolContext *context)
{
	assert(0);
	return DECODE_ERROR;
}

void KVDataProtocolFactory::DeleteProtocol(uint32_t protocol_type, void *protocol)
{
	assert(protocol != NULL);
	if(m_Memory != NULL)
	{
		m_Memory->Free(protocol, sizeof(KVData));
	}
	else
	{
		SystemMemory sys_memory;
		sys_memory.Free(protocol, sizeof(KVData));
	}
}

}//namespace
