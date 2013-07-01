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
	char *data = context->Buffer+context->Size;

	data -= 4;
	if(strncmp(data, "\r\n\r\n", 4) != 0)
		return DECODE_DATA;

	HttpRequest req;
	if(ParseRequest(req, context->Buffer, context->Size) == false)
		return DECODE_ERROR;
	return DECODE_SUCC;
}

void HttpReqProtocolFactory::DeleteProtocol(uint32_t protocol_type, void *protocol)
{
	////Add Your Code Here

	return ;
}


bool HttpReqProtocolFactory::ParseRequest(HttpRequest &request, char *buffer, uint32_t size)
{
	if(strncasecmp(buffer, "GET ", 4) == 0)
	{
		request.req_type = REQ_TYPE_GET;
		request.url = buffer+4;
	}
	else if(strncasecmp(buffer, "HEAD ", 5) == 0)
	{
		request.req_type = REQ_TYPE_HEAD;
		request.url = buffer+5;
	}
	else
		return false;

	request.version = request.url;
	while(*request.version!=' ' && *request.version!='\r' && *request.version!='\n')
		++request.version;
	if(request.version[0]==' ')
	{
		request.url_len = request.version-request.url;
		++request.version;
	}
	else
		return false;

	request.version_len = 0;
	while(*request.version!='\r' && *request.version!='\n')
		++request.version_len;

	return true;
}
