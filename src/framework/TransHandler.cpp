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
#include <sys/socket.h>

#include "Socket.h"
#include "TransHandler.h"
#include "IAppInterface.h"

namespace easynet
{
IMPL_LOGGER(TransHandler, logger);

void TransHandler::OnEventError(int32_t fd, uint64_t nowtime_ms, ERROR_CODE code)
{
	if(code == ECODE_ERROR)
		LOG_ERROR(logger, "socket error. fd="<<fd);
	else if(code == ECODE_TIMEOUT)
		LOG_ERROR(logger, "socket timeout. fd="<<fd);
	else if(code == ECODE_CLOSE)
		LOG_DEBUG(logger, "peer close socket gracefully. fd="<<fd);
	else
		LOG_WARN(logger, "other error code. ecode="<<code<<",fd=<<fd");

	ProtocolContext *context;
	FDMap::iterator it = m_RecvFdMap.find(fd);
	if(it != m_RecvFdMap.end())    //删除正在接收的协议
	{
		m_RecvFdMap.erase(it);
		context = it->second;
		m_AppInterface->DeleteProtocolContext(context);
	}

	it = m_SendFdMap.find(fd);
	if(it != m_SendFdMap.end())
	{
		m_SendFdMap.erase(it);
		m_AppInterface->OnSendError(fd, it->second);    //删除正在发送的协议
	}
	while((context = m_AppInterface->GetSendProtocol(fd)) != NULL)    //删除等待发送队列中的协议
		m_AppInterface->OnSendError(fd, context);

	m_AppInterface->OnSocketFinished(fd);
}

//可读事件
ERROR_CODE TransHandler::OnEventRead(int32_t fd, uint64_t now_time)
{
	LOG_DEBUG(logger, "on event_read. fd="<<fd);

	IProtocolFactory *protocol_factory = m_AppInterface->GetProtocolFactory();
	assert(protocol_factory != NULL);

	ProtocolContext *context = NULL;
	FDMap::iterator it = m_RecvFdMap.find(fd);
	if(it == m_RecvFdMap.end())
	{
		context = m_AppInterface->NewProtocolContext();
		assert(context!=NULL && context->bytebuffer!=NULL);

		context->type = DTYPE_INVALID;
		context->header_size = protocol_factory->HeaderSize();
		context->fd = fd;
		context->timeout_ms = m_AppInterface->GetRecvTimeoutMS();
		if(context->timeout_ms > 0)
			context->expire_time = now_time+context->timeout_ms;

		std::pair<FDMap::iterator, bool> result = m_RecvFdMap.insert(std::make_pair(fd, context));
		if(result.second == false)
		{
			LOG_ERROR(logger, "recv new data packet but insert to map failed. fd="<<fd);
			m_AppInterface->DeleteProtocolContext(context);
			return ECODE_ERROR;
		}
		it = result.first;

		LOG_DEBUG(logger, "receive a new data packet. type="<<context->type<<", expect_size="<<context->header_size<<", fd="<<fd);
	}
	else
	{
		context = it->second;
		assert(context != NULL);

		//超时检查
		if(context->timeout_ms>0 && context->expire_time<now_time)
		{
			LOG_WARN(logger, "receive protocol data timeout. fd="<<fd<<", timeout_ms="<<context->timeout_ms);
			m_RecvFdMap.erase(it);
			m_AppInterface->DeleteProtocolContext(context);
			return ECODE_ERROR;
		}
		LOG_DEBUG(logger, "continue to receive data packet. data_type="<<context->type<<", expect_size="<<context->header_size+context->body_size<<", cur_size="<<(context->bytebuffer)->m_Size<<", fd="<<fd);
	}

	if(context->type == DTYPE_INVALID)  //协议类型未知,需要解码协议头来判断是文本协议还是二进制协议
	{
		ByteBuffer *byte_buffer = context->bytebuffer;
		if(byte_buffer->m_Capacity < context->header_size)
		{
			bool result = byte_buffer->Enlarge(context->header_size);
			assert(result == true);
		}
		char *buffer         = byte_buffer->m_Buffer+byte_buffer->m_Size;
		uint32_t buffer_size = byte_buffer->m_Capacity-byte_buffer->m_Size;
		uint32_t need_size   = context->header_size-byte_buffer->m_Size;
		int32_t recv_size    = ReadData(fd, buffer, buffer_size, need_size);
		if(recv_size < 0)
		{
			if(recv_size == -1)
				LOG_ERROR(logger, "read data error. expect_size="<<context->header_size<<", cur_size="<<byte_buffer->m_Size<<", fd="<<fd);
			else
				LOG_DEBUG(logger, "read data error. expect_size="<<context->header_size<<", cur_size="<<byte_buffer->m_Size<<", fd="<<fd);
			m_RecvFdMap.erase(it);
			m_AppInterface->DeleteProtocolContext(context);
			return recv_size==-1?ECODE_ERROR:ECODE_CLOSE;
		}
		byte_buffer->m_Size += recv_size;

		DecodeResult result = protocol_factory->DecodeHeader(byte_buffer->m_Buffer, context->type, context->body_size);
		if(result == DECODE_ERROR)
		{
			LOG_ERROR(logger, "decode header error. expect_size="<<context->header_size<<", cur_size="<<byte_buffer->m_Size<<", fd="<<fd);
			m_RecvFdMap.erase(it);
			m_AppInterface->DeleteProtocolContext(context);
			return ECODE_ERROR;
		}
		else if(result == DECODE_DATA)
		{
			LOG_DEBUG(logger, "wait for more header data. expect_size="<<context->header_size<<", cur_size="<<byte_buffer->m_Size<<", fd="<<fd);
			return ECODE_PENDING;
		}

		//解码头部成功
		assert(context->type != DTYPE_INVALID);
		LOG_DEBUG(logger, "decode header succ. data_type="<<context->type<<", header_size="<<context->header_size<<", body_size="<<context->body_size<<", cur_size="<<byte_buffer->m_Size<<", fd="<<fd);
	}

	//读可能剩下的二进制协议体数据
	if(context->type==DTYPE_BIN && context->bytebuffer->m_Size<context->header_size+context->body_size)
	{
		ByteBuffer *byte_buffer = context->bytebuffer;
		if(byte_buffer->m_Capacity < context->header_size+context->body_size)
		{
			bool result = byte_buffer->Enlarge(context->header_size+context->body_size);
			assert(result == true);
		}
		char *buffer         = byte_buffer->m_Buffer+byte_buffer->m_Size;
		uint32_t buffer_size = byte_buffer->m_Capacity-byte_buffer->m_Size;
		uint32_t need_size   = context->header_size+context->body_size-byte_buffer->m_Size;
		int32_t recv_size    = ReadData(fd, buffer, buffer_size, need_size);
		if(recv_size < 0)
		{
			if(recv_size == -1)
				LOG_ERROR(logger, "read data error. expect_size="<<context->header_size<<", cur_size="<<byte_buffer->m_Size<<", fd="<<fd);
			else
				LOG_DEBUG(logger, "read data error. expect_size="<<context->header_size<<", cur_size="<<byte_buffer->m_Size<<", fd="<<fd);
			m_RecvFdMap.erase(it);
			m_AppInterface->DeleteProtocolContext(context);
			return recv_size==-1?ECODE_ERROR:ECODE_CLOSE;
		}
		byte_buffer->m_Size += recv_size;
	}

	//解码
	DecodeResult result;
	if(context->type == DTYPE_BIN)
		result = protocol_factory->DecodeBinBody(context);
	else if (context->type == DTYPE_TEXT)
		result = protocol_factory->DecodeBinBody(context);
	if(result == DECODE_ERROR)
	{
		LOG_ERROR(logger, "decode body error. data_type="<<context->type<<", header_size="<<context->header_size<<", body_size="<<context->body_size<<", cur_size="<<context->bytebuffer->m_Size<<", fd="<<fd);
		m_RecvFdMap.erase(it);
		m_AppInterface->DeleteProtocolContext(context);
		return ECODE_ERROR;
	}
	else if(result == DECODE_DATA)
	{
		LOG_DEBUG(logger, "wait for more body data. data_type="<<context->type<<", header_size="<<context->header_size<<", body_size="<<context->body_size<<", cur_size="<<context->bytebuffer->m_Size<<", fd="<<fd);
		return ECODE_PENDING;
	}

	LOG_DEBUG(logger, "decode body succ. data_type="<<context->type<<", fd="<<fd);
	m_RecvFdMap.erase(it);

	bool detach_context = false;
	bool hand_result = m_AppInterface->OnReceiveProtocol(fd, context, detach_context);
	if(!hand_result || !detach_context)
		m_AppInterface->DeleteProtocolContext(context);
	return hand_result==true?ECODE_SUCC:ECODE_ERROR;
}

//可写事件
ERROR_CODE TransHandler::OnEventWrite(int32_t fd, uint64_t now_time)
{
	LOG_DEBUG(logger, "on event_write. fd="<<fd);

	FDMap::iterator it = m_SendFdMap.find(fd);
	while(true)
	{
		ProtocolContext *context = NULL;
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
		if(context->timeout_ms>0 && context->expire_time<now_time)
		{
			m_AppInterface->OnSendTimeout(fd, context);
			if(it != m_SendFdMap.end())
				m_SendFdMap.erase(it);
			continue;
		}
		//发送数据
		ByteBuffer *byte_buffer = context->bytebuffer;
		char *buffer = byte_buffer->m_Buffer+context->send_size;
		uint32_t need_size = byte_buffer->m_Size-context->send_size;
		int32_t send_size = Socket::Send(fd, buffer, need_size);
		if(send_size == -1)    //socket有问题,发送失败,返回false
		{
			LOG_ERROR(logger, "send data error. context="<<context<<", data_type="<<context->type<<", total_size="<<byte_buffer->m_Size<<", send_size="<<context->send_size<<", fd="<<fd);
			if(it == m_SendFdMap.end())    //保存为正在发送的协议,由OnEventError方法处理
				m_SendFdMap.insert(std::make_pair(fd, context));
			return ECODE_ERROR;
		}

		context->send_size += send_size;
		if(context->send_size >= byte_buffer->m_Size)  //发送完成
		{
			LOG_DEBUG(logger, "send data finished. context="<<context<<", data_type="<<context->type<<", total_size="<<byte_buffer->m_Size<<", send_size="<<context->send_size<<", fd="<<fd);
			if(it != m_SendFdMap.end())
			{
				m_SendFdMap.erase(it);
				it = m_SendFdMap.end();
			}
			m_AppInterface->OnSendSucc(fd, context);
			continue;
		}

		//只发送了一半数据,等待下次可写事件
		LOG_INFO(logger, "send data partly. context="<<context<<", data_type="<<context->type<<", total_size="<<byte_buffer->m_Size<<", send_size="<<context->send_size<<", fd="<<fd);
		if(it == m_SendFdMap.end())    //保存为正在发送的数据
			m_SendFdMap.insert(std::make_pair(fd, context));

		return ECODE_PENDING;
	}
	return ECODE_SUCC;
}

int32_t TransHandler::ReadData(int32_t fd, char *buffer, uint32_t buffer_size, uint32_t need_size)
{
	assert(buffer_size >= need_size);
	int32_t recv_size = Socket::Recv(fd, buffer, need_size);
	if(recv_size == 0)
	{
		recv_size = -2;
	}
	else if(recv_size < 0)
	{
		if(errno==EAGAIN || errno==EINTR || errno==EWOULDBLOCK)
		{
			LOG_DEBUG(logger, "no data for reading. error="<<errno<<"("<<strerror(errno)<<").");
			recv_size = 0;
		}
		else
			LOG_ERROR(logger, "read data error. error="<<errno<<"("<<strerror(errno)<<").");
	}
	return recv_size;
}


}//namespace
