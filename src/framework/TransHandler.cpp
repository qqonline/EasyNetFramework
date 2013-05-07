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

	FDMap::iterator it = m_FdMap.find(fd);
	if(it != m_FdMap.end())
	{
		m_ProtocolFactory->DeleteContext(it->second);
		m_FdMap.erase(it);
	}

	m_AppInterface->OnSocketTimeout(fd);
	return true;
}

//可读事件
bool TransHandler::OnEventRead(int32_t fd)
{
	LOG_DEBUG(logger, "on event_read. fd="<<fd);

	RecvContext *context = NULL;
	FDMap::iterator it = m_FdMap.find(fd);
	if(it == m_FdMap.end())
	{
		context = m_ProtocolFactory->NewContext();
		if(context == NULL)
		{
			LOG_ERROR(logger, "get protocol context failed. maybe out of memory. fd="<<fd);
			return false;
		}
		context->fd = fd;
		std::pair<FDMap::iterator, bool> result = m_FdMap.insert(std::make_pair(fd, context));
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
		const uint32_t DATA_HEADER_SIZE = m_ProtocolFactory->DataHeaderSize();    //伪协议头长度
		assert(context->cur_data_size < DATA_HEADER_SIZE);

		if(ReadData(context->buffer, context->buffer_size, DATA_HEADER_SIZE-context->cur_data_size) == -1)
		{
			LOG_ERROR(logger, "read temp_header data error. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_FdMap.erase(it);
			return false;
		}
		DecodeResult result = m_ProtocolFactory->DecodeDataType(context);
		if(result == DECODE_ERROR)
		{
			LOG_ERROR(logger, "decode data type error. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_FdMap.erase(it);
			return false;
		}
		else if(result == DECODE_DATA)
		{
			LOG_DEBUG(logger, "wait for more temp_header data. temp_header_size="<<DATA_HEADER_SIZE<<" cur_size="<<context->cur_data_size<<" fd="<<fd);
			return true;
		}

		assert(context->data_type != DTYPE_INVALID);
		LOG_DEBUG(logger, "decode data type succ. data_type="<<context->data_type<<" header_size="<<context->header_size<<" body_size="<<context->body_size);
	}

	if(context->data_type==DTYPE_BIN && context->body_size==0)  //解码二进制协议头
	{
		assert(context->header_size > 0);
		assert(context->cur_header_size <= context->header_size);
		uint32_t need_size = context->header_size-context->cur_header_size;
		if(need_size>0 && ReadData(context->buffer, context->buffer_size, need_size)==-1)    //读协议头数据
		{
			LOG_ERROR(logger, "read bin_header data error. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_FdMap.erase(it);
			return false;
		}

		DecodeResult decode_result = m_ProtocolFactory->DecodeBinHeader(context);
		if(decode_result == DECODE_ERROR)
		{
			LOG_ERROR(logger, "decode bin_header error. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_FdMap.erase(it);
			return false;
		}
		else if(decode_result == DECODE_DATA)
		{
			LOG_DEBUG(logger, "wait for more bin_header data. header_size="<<context->header_size<<" cur_header_size="<<context->cur_header_size<<" fd="<<fd);
			return true;
		}

		LOG_DEBUG(logger, "decode bin_header succ. body_size="<<context->body_size);
		if(context->body_size == 0)    //允许空包
		{
			LOG_INFO(logger, "receive a empty bin_packet. do nothing. fd="<<fd);
			m_ProtocolFactory->DeleteContext(context);
			m_FdMap.erase(it);
			return true;
		}
	}

	//解码二进制/文本协议的包体
	assert(context->body_size > 0);
	if(ReadData(context->buffer, context->buffer_size, context->body_size-context->cur_body_size) == -1)      //读协议体数据
	{
		LOG_ERROR(logger, "read body data error. fd="<<fd);
		m_ProtocolFactory->DeleteContext(context);
		m_FdMap.erase(it);
		return false;
	}

	DecodeResult decode_result = DECODE_ERROR;
	if(context->data_type == DTYPE_BIN)
		decode_result = m_ProtocolFactory->DecodeBinBody(context);
	else if (context->data_type == DTYPE_TEXT)
		decode_result = m_ProtocolFactory->DecodeBinBody(context);
	if(decode_result == DECODE_ERROR)
	{
		LOG_ERROR(logger, "decode body error. fd="<<fd);
		m_ProtocolFactory->DeleteContext(context);
		m_FdMap.erase(it);
		return false;
	}
	else if(decode_result == DECODE_DATA)
	{
		LOG_DEBUG(logger, "wait for more body data. body_size="<<context->body_size<<" cur_body_size="<<context->cur_body_size<<" fd="<<fd);
		return true;
	}

	LOG_DEBUG(logger, "decode body succ. data_type="<<context->data_type<<" fd="<<fd);
	m_FdMap.erase(it);

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
		SendContext *context = NULL;
		m_AppInterface->GetSendProtocol(fd, &context);
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

	FDMap::iterator it = m_FdMap.find(fd);
	if(it != m_FdMap.end())
	{
		m_ProtocolFactory->DeleteContext(it->second);
		m_FdMap.erase(it);
	}

	m_AppInterface->OnSocketError(fd);
	return true;
}

}//namespace
