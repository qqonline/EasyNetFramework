/*
 * HttpReqProtocolFactory.cpp
 *
 *  Created on: 2013-06-24
 *      Author: tim
 */

#include "HttpReqProtocolFactory.h"

#include "string.h"

uint32_t HttpReqProtocolFactory::HeaderSize()
{
	////Add Your Code Here

	return 4;
}

DecodeResult HttpReqProtocolFactory::DecodeHeader(const char *buffer, DataType &type, uint32_t &body_size)
{
	////Add Your Code Here
	if(strncasecmp(buffer, "GET ", 4) != 0)
		return DECODE_ERROR;
	type = DTYPE_TEXT;
	body_size = 1024;  //http请求串最大长度

	return DECODE_SUCC;
}

void HttpReqProtocolFactory::EncodeHeader(char *buffer, uint32_t body_size)
{
	////Add Your Code Here

	return;
}

DecodeResult HttpReqProtocolFactory::DecodeBinBody(ProtocolContext *context)
{
	////Add Your Code Here

	return DECODE_ERROR;
}

DecodeResult HttpReqProtocolFactory::DecodeTextBody(ProtocolContext *context)
{
	////Add Your Code Here
	ByteBuffer *bytebuffer = context->bytebuffer;

	char *data = bytebuffer->m_Buffer+bytebuffer->m_Size;

	data -= 4;
	if(strncmp(data, "\r\n\r\n", 4) != 0)
		return DECODE_DATA;

	return DECODE_SUCC;
}

void HttpReqProtocolFactory::DeleteProtocol(uint32_t protocol_type, void *protocol)
{
	////Add Your Code Here

	return ;
}

