/*
 * TransHandler.cpp
 *
 *  Created on: 2013-5-3
 *      Author: LiuYongJin
 */
#include <stdlib.h>
#include <sys/time.h>

#include "TransHandler.h"

namespace easynet
{
IMPL_LOGGER(TransHandler, logger);

#define GetCurTime(now) do{              \
	struct timeval tv;                    \
	gettimeofday(&tv, NULL);              \
	now = tv.tv_sec*1000+tv.tv_usec/1000; \
}while(0)

bool TransHandler::OnTimeout(int32_t fd)
{
	LOG_DEBUG(logger, "on time_out. fd="<<fd);

	FDMap::iterator it = m_RecvFdMap.find(fd);
	if(it != m_RecvFdMap.end())
	{
		m_ProtocolFactory->DeleteContext(it->second);
		m_RecvFdMap.erase(it);
	}

	m_AppInterface->OnSocketTimeout(fd);
	return true;
}

//可读事件
bool TransHandler::OnEventRead(int32_t fd)
{
	LOG_DEBUG(logger, "on event_read. fd="<<fd);

	ProtocolContext *context = NULL;
	FDMap::iterator it = m_RecvFdMap.find(fd);
	if(it == m_RecvFdMap.end())
	{
		context = m_ProtocolFactory->NewRecvContext();
		if(context == NULL)
		{
			LOG_ERROR(logger, "get protocol context failed. maybe out of memory. fd="<<fd);
			return false;
		}
		context->fd = fd;
		std::pair<FDMap::iterator, bool> result = m_RecvFdMap.insert(std::make_pair(fd, context));
		if(result.second == false)
		{
			LOG_ERROR(logger, "insert to map failed. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			return false;
		}
		it = result.first;
	}
	context = it->second; assert(context != NULL);

	//检查数据类型:二进制协议/文本协议
	if(context->data_type == DTYPE_INVALID)
	{
		assert(context->cur_data_size < context->data_header_size);

		char *buffer         = context->buffer+context->cur_data_size;
		uint32_t buffer_size = context->buffer_size-context->cur_data_size;
		uint32_t need_size   = context->data_header_size-context->cur_data_size;
		int32_t recv_size    = ReadData(buffer, buffer_size, need_size);
		if(recv_size== -1)
		{
			LOG_ERROR(logger, "read temp_header data error. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_RecvFdMap.erase(it);
			return false;
		}
		context->cur_data_size += recv_size;

		DecodeResult result = m_ProtocolFactory->DecodeDataType(context);
		if(result == DECODE_ERROR)
		{
			LOG_ERROR(logger, "decode data type error. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_RecvFdMap.erase(it);
			return false;
		}
		else if(result == DECODE_DATA)
		{
			LOG_DEBUG(logger, "wait for more temp_header data. temp_header_size="<<context->data_header_size<<" cur_size="<<context->cur_data_size<<" fd="<<fd);
			return true;
		}

		if(context->data_type == DTYPE_INVALID)
			assert(0);
		else if(context->data_type==DTYPE_BIN)
		{
			assert(context->header_size>0);
			context->body_size = 0;
		}
		else
		{
			assert(context->body_size > 0);
			context->header_size = 0;
		}

		LOG_DEBUG(logger, "decode data type succ. data_type="<<context->data_type<<" header_size="<<context->header_size<<" body_size="<<context->body_size);
	}

	if(context->data_type==DTYPE_BIN && context->cur_data_size<=context->header_size)  //解码二进制协议头
	{
		char *buffer         = context->buffer+context->cur_data_size;
		uint32_t buffer_size = context->buffer_size-context->cur_data_size;
		uint32_t need_size   = context->header_size-context->cur_data_size;
		int32_t recv_size    = need_size>0?ReadData(buffer, buffer_size, need_size):0;
		if(recv_size== -1)
		{
			LOG_ERROR(logger, "read bin_header data error. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_RecvFdMap.erase(it);
			return false;
		}
		context->cur_data_size += recv_size;

		DecodeResult decode_result = m_ProtocolFactory->DecodeBinHeader(context);
		if(decode_result == DECODE_ERROR)
		{
			LOG_ERROR(logger, "decode bin_header error. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_RecvFdMap.erase(it);
			return false;
		}
		else if(decode_result == DECODE_DATA)
		{
			LOG_DEBUG(logger, "wait for more bin_header data. header_size="<<context->header_size<<" cur_header_size="<<context->cur_data_size<<" fd="<<fd);
			return true;
		}

		LOG_DEBUG(logger, "decode bin_header succ. body_size="<<context->body_size);
		if(context->body_size == 0)    //允许空包
		{
			LOG_INFO(logger, "receive a empty bin_packet. do nothing. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_RecvFdMap.erase(it);
			return true;
		}
	}

	//解码二进制/文本协议的包体
	assert(context->body_size > 0);
	char *buffer         = context->buffer+context->cur_data_size;
	uint32_t buffer_size = context->buffer_size-context->cur_data_size;
	uint32_t need_size   = context->header_size+context->body_size-context->cur_data_size;
	int32_t recv_size    = need_size>0?ReadData(buffer, buffer_size, need_size):0;
	if(recv_size== -1)
	{
		LOG_ERROR(logger, "read body data error. fd="<<fd);
		m_ProtocolFactory->DeleteContext(context);
		m_RecvFdMap.erase(it);
		return false;
	}
	context->cur_data_size += recv_size;

	DecodeResult decode_result = DECODE_ERROR;
	if(context->data_type == DTYPE_BIN)
		decode_result = m_ProtocolFactory->DecodeBinBody(context);
	else if (context->data_type == DTYPE_TEXT)
		decode_result = m_ProtocolFactory->DecodeBinBody(context);
	if(decode_result == DECODE_ERROR)
	{
		LOG_ERROR(logger, "decode body error. fd="<<fd);
		m_ProtocolFactory->DeleteContext(context);
		m_RecvFdMap.erase(it);
		return false;
	}
	else if(decode_result == DECODE_DATA)
	{
		LOG_DEBUG(logger, "wait for more body data. body_size="<<context->body_size<<" cur_body_size="<<context->cur_data_size-context->header_size<<" fd="<<fd);
		return true;
	}

	LOG_DEBUG(logger, "decode body succ. data_type="<<context->data_type<<" fd="<<fd);
	m_RecvFdMap.erase(it);

	bool detach_context = false;
	bool result = m_AppInterface->OnReceiveProtocol(fd, context, detach_context);
	if(!result || !detach_context)
		m_ProtocolFactory->DeleteContext(context);
	return result;
}

//可写事件
bool TransHandler::onEventWrite(int32_t fd)
{
	LOG_DEBUG(logger, "on event_write. fd="<<fd);

	uint64_t time_now;
	GetCurTime(time_now);

	while(true)
	{
		ProtocolContext *context = m_AppInterface->GetSendProtocol(fd);
		if(context == NULL)
			break;
		if(context->expire_time < time_now)
		{
			m_AppInterface->OnSendTimeout(fd, context);
			continue;
		}

		//TODO
		//sent protocol
		m_AppInterface->OnSendSucc(fd, context);
		m_AppInterface->OnSendError(fd, context);
	}
	return true;
}

//错误事件
bool TransHandler::OnEventError(int32_t fd)
{
	LOG_DEBUG(logger, "on event_error. fd="<<fd);

	FDMap::iterator it = m_RecvFdMap.find(fd);
	if(it != m_RecvFdMap.end())
	{
		m_ProtocolFactory->DeleteContext(it->second);
		m_RecvFdMap.erase(it);
	}

	m_AppInterface->OnSocketError(fd);
	return true;
}

}//namespace
