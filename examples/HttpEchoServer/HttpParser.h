/*
 * HttpParser.h
 *
 *  Created on: 2013-7-1
 *      Author: tim
 */

#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_

#include <string.h>
#include <stdint.h>

typedef enum _http_req_type
{
	REQ_TYPE_UNKNOW,
	REQ_TYPE_GET,
	REQ_TYPE_HEAD
}HttpReqType;

class HttpRequest
{
public:
	HttpReqType req_type;
	char *url;
	uint32_t url_len;
	char *version;
	uint32_t version_len;
};

class HttpParser
{
public:
	static bool ParseRequest(HttpRequest &request, char *buffer, uint32_t size);
};

inline
bool HttpParser::ParseRequest(HttpRequest &request, char *buffer, uint32_t size)
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
	while(request.version[request.version_len]!='\r' && request.version[request.version_len]!='\n')
		++request.version_len;

	return true;
}

#endif


