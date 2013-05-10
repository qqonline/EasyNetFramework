/*
 * TransHandler.cpp
 *
 *  Created on: 2013-5-3
 *      Author: LiuYongJin
 */
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

#include "TransHandler.h"

namespace easynet
{
IMPL_LOGGER(TransHandler, logger);

bool TransHandler::OnTimeout(int32_t fd, uint64_t now_time)
{
	LOG_DEBUG(logger, "on time_out. fd="<<fd);

	IProtocolFactory *protocol_factory = m_AppInterface->GetProtocolFactory();
	assert(protocol_factory != NULL);

	FDMap::iterator it = m_RecvFdMap.find(fd);
	if(it != m_RecvFdMap.end())    //删除正在接收的协议
	{
		m_RecvFdMap.erase(it);
		protocol_factory->DeleteContext(it->second);
	}

	it = m_SendFdMap.find(fd);
	if(it != m_SendFdMap.end())
	{
		m_SendFdMap.erase(it);
		m_AppInterface->OnSendError(fd, it->second);    //删除正在发送的协议
		ProtocolContext *context;
		while((context = m_AppInterface->GetSendProtocol(fd)) != NULL)    //删除等待发送队列中的协议
			m_AppInterface->OnSendError(fd, it->second);
	}

	m_AppInterface->OnSocketTimeout(fd);
	return true;
}


//错误事件
bool TransHandler::OnEventError(int32_t fd, uint64_t now_time)
{
	LOG_DEBUG(logger, "on event_error. fd="<<fd);

	IProtocolFactory *protocol_factory = m_AppInterface->GetProtocolFactory();
	assert(protocol_factory != NULL);

	FDMap::iterator it = m_RecvFdMap.find(fd);
	if(it != m_RecvFdMap.end())    //删除正在接收的协议
	{
		m_RecvFdMap.erase(it);
		protocol_factory->DeleteContext(it->second);
	}

	it = m_SendFdMap.find(fd);
	if(it != m_SendFdMap.end())
	{
		m_SendFdMap.erase(it);
		m_AppInterface->OnSendError(fd, it->second);    //删除正在发送的协议
		ProtocolContext *context;
		while((context = m_AppInterface->GetSendProtocol(fd)) != NULL)    //删除等待发送队列中的协议
			m_AppInterface->OnSendError(fd, it->second);
	}

	m_AppInterface->OnSocketError(fd);
	return true;
}

//可读事件
bool TransHandler::OnEventRead(int32_t fd, uint64_t now_time)
{
	LOG_DEBUG(logger, "on event_read. fd="<<fd);

	IProtocolFactory *protocol_factory = m_AppInterface->GetProtocolFactory();
	assert(protocol_factory != NULL);

	ProtocolContext *context = NULL;
	FDMap::iterator it = m_RecvFdMap.find(fd);
	if(it == m_RecvFdMap.end())
	{
		context = protocol_factory->NewRecvContext();
		if(context == NULL)
		{
			LOG_ERROR(logger, "get protocol context failed. maybe out of memory. fd="<<fd);
			return false;
		}
		context->fd = fd;
		context->time_out = m_AppInterface->GetRecvTimeout();
		if(context->time_out > 0)
			context->expire_time = now_time+context->time_out;

		std::pair<FDMap::iterator, bool> result = m_RecvFdMap.insert(std::make_pair(fd, context));
		if(result.second == false)
		{
			LOG_ERROR(logger, "insert to map failed. fd="<<fd);
			protocol_factory->DeleteContext(context);
			return false;
		}
		it = result.first;
	}
	context = it->second; assert(context != NULL);

	if(context->time_out > 0)    //超时检查
	{
		if(context->expire_time < now_time)
		{
			LOG_ERROR(logger, "receive protocol data timeout. fd="<<fd<<" time_out="<<context->time_out);
			m_RecvFdMap.erase(it);
			protocol_factory->DeleteContext(context);
			return false;
		}
	}

	//检查数据类型:二进制协议/文本协议
	if(context->data_type == DTYPE_INVALID)
	{
		assert(context->cur_data_size < context->data_header_size);

		char *buffer         = context->buffer+context->cur_data_size;
		uint32_t buffer_size = context->buffer_size-context->cur_data_size;
		uint32_t need_size   = context->data_header_size-context->cur_data_size;
		int32_t recv_size    = ReadData(fd, buffer, buffer_size, need_size);
		if(recv_size== -1)
		{
			LOG_ERROR(logger, "read temp_header data error. fd="<<fd);
			m_RecvFdMap.erase(it);
			protocol_factory->DeleteContext(context);
			return false;
		}
		context->cur_data_size += recv_size;

		DecodeResult result = protocol_factory->DecodeDataType(context);
		if(result == DECODE_ERROR)
		{
			LOG_ERROR(logger, "decode data type error. fd="<<fd);
			m_RecvFdMap.erase(it);
			protocol_factory->DeleteContext(context);
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
		int32_t recv_size    = need_size>0?ReadData(fd, buffer, buffer_size, need_size):0;
		if(recv_size== -1)
		{
			LOG_ERROR(logger, "read bin_header data error. fd="<<fd);
			m_RecvFdMap.erase(it);
			protocol_factory->DeleteContext(context);
			return false;
		}
		context->cur_data_size += recv_size;

		DecodeResult decode_result = protocol_factory->DecodeBinHeader(context);
		if(decode_result == DECODE_ERROR)
		{
			LOG_ERROR(logger, "decode bin_header error. fd="<<fd);
			m_RecvFdMap.erase(it);
			protocol_factory->DeleteContext(context);
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
			m_RecvFdMap.erase(it);
			protocol_factory->DeleteContext(context);
			return true;
		}
	}

	//解码二进制/文本协议的包体
	assert(context->body_size > 0);
	char *buffer         = context->buffer+context->cur_data_size;
	uint32_t buffer_size = context->buffer_size-context->cur_data_size;
	uint32_t need_size   = context->header_size+context->body_size-context->cur_data_size;
	int32_t recv_size    = need_size>0?ReadData(fd, buffer, buffer_size, need_size):0;
	if(recv_size== -1)
	{
		LOG_ERROR(logger, "read body data error. fd="<<fd);
		m_RecvFdMap.erase(it);
		protocol_factory->DeleteContext(context);
		return false;
	}
	context->cur_data_size += recv_size;

	DecodeResult decode_result = DECODE_ERROR;
	if(context->data_type == DTYPE_BIN)
		decode_result = protocol_factory->DecodeBinBody(context);
	else if (context->data_type == DTYPE_TEXT)
		decode_result = protocol_factory->DecodeBinBody(context);
	if(decode_result == DECODE_ERROR)
	{
		LOG_ERROR(logger, "decode body error. fd="<<fd);
		m_RecvFdMap.erase(it);
		protocol_factory->DeleteContext(context);
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
		protocol_factory->DeleteContext(context);
	return result;
}

//可写事件
bool TransHandler::onEventWrite(int32_t fd, uint64_t now_time)
{
	LOG_DEBUG(logger, "on event_write. fd="<<fd);

	IProtocolFactory *protocol_factory = m_AppInterface->GetProtocolFactory();
	assert(protocol_factory != NULL);

	ProtocolContext *context = NULL;
	FDMap::iterator it = m_SendFdMap.find(fd);
	while(true)
	{
		//检查是否有正在发送的协议
		if(it != m_SendFdMap.end())
		{
			context = it->second;
			assert(context != NULL);
		}
		//没有正在发送的协议,再检查待发送队列是否有需要发送的协议
		if(context == NULL)
			context = m_AppInterface->GetSendProtocol(fd);
		if(context == NULL)
			break;
		//发送超时
		if(context->time_out>0 && context->expire_time<now_time)
		{
			m_AppInterface->OnSendTimeout(fd, context);
			continue;
		}
		//需要解码
		if(it==m_SendFdMap.end() && !protocol_factory->EncodeProtocol(context->protocol, context->protocol_type, context->buffer, context->buffer_size))    //编码错误(数据有问题?)
		{
			LOG_ERROR(logger, "encode protocol error. context="<<context<<" data_type="<<context->data_type<<" protocol_type="<<context->protocol_type<<" fd="<<fd);
			m_AppInterface->OnSendError(fd, context);
			continue;
		}
		//发送数据
		char *buffer = context->buffer+context->cur_data_size;
		uint32_t need_size = context->body_size+context->header_size-context->cur_data_size;
		int32_t send_size = send(fd, buffer, need_size, 0);
		if(send_size == -1)    //socket有问题,发送失败,返回false
		{
			LOG_ERROR(logger, "send data error. context="<<context<<" data_type="<<context->data_type<<" protocol_type="<<context->protocol_type<<" fd="<<fd);
			if(it == m_SendFdMap.end())    //保存为正在发送的协议,由OnEventError方法处理
				m_SendFdMap.insert(std::make_pair(fd, context));
			return false;
		}

		context->cur_data_size += send_size;
		if(context->cur_data_size >= context->header_size+context->body_size)  //发送完成
		{
			LOG_DEBUG(logger, "send data finised. context="<<context
						<<" data_type="<<context->data_type
						<<" protocol_type="<<context->protocol_type
						<<" total size="<<context->body_size+context->header_size
						<<" send size="<<context->cur_data_size
						<<" fd="<<fd);
			if(it != m_SendFdMap.end())
			{
				m_SendFdMap.erase(it);
				it = m_SendFdMap.end();
			}
			m_AppInterface->OnSendSucc(fd, context);
			continue;
		}

		LOG_INFO(logger, "send data partly. context="<<context
					<<" data_type="<<context->data_type
					<<" protocol_type="<<context->protocol_type
					<<" total size="<<context->body_size+context->header_size
					<<" send size="<<context->cur_data_size
					<<" fd="<<fd);
		if(it == m_SendFdMap.end())    //保存为正在发送的数据
			m_SendFdMap.insert(std::make_pair(fd, context));
		break;
	}
	return true;
}

int32_t TransHandler::ReadData(int32_t fd, char *buffer, uint32_t buffer_size, uint32_t need_size)
{
	assert(buffer_size >= need_size);
	int32_t recv_size = recv(fd, buffer, need_size, 0);
	if(recv_size == 0)
	{
		LOG_DEBUG(logger, "peer close socket gracefully. fd="<<fd);
		recv_size = -1;
	}
	else if(recv_size < 0)
	{
		if(errno==EAGAIN || errno==EINTR || errno==EWOULDBLOCK)
		{
			LOG_DEBUG(logger, "no data for reading. error="<<errno<<"("<<strerror(errno)<<")");
			recv_size = 0;
		}
		else
			LOG_ERROR(logger, "read data error. error="<<errno<<"("<<strerror(errno)<<")");
	}
	return recv_size;
}


}//namespace
