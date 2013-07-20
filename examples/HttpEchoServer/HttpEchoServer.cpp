/*
 * HttpEchoServer.cpp
 *
 *  Created on: 2013-06-24
 *      Author: tim
 */

#include "HttpEchoServer.h"

#include "Socket.h"
#include "HttpReqProtocolFactory.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

IMPL_LOGGER(HttpEchoServer, logger);

HttpEchoServer::HttpEchoServer()
{
	m_ProtocolFactory = (IProtocolFactory*)new HttpReqProtocolFactory(GetMemory());
}

bool HttpEchoServer::Start()
{
	//Add Your Code Here
	if(Listen(8080) == false)
	{
		LOG_ERROR(logger, "listen on port=8080 error.");
		return false;
	}

	LOG_INFO(logger, "server listen on port=8080 succ.");

	IEventServer *event_server = GetEventServer();
	event_server->RunLoop();

	return true;
}

int32_t HttpEchoServer::GetSocketRecvTimeout()
{
	return 3000;   //接收时间3s超时
}

int32_t HttpEchoServer::GetSocketIdleTimeout()
{
	return 3000;
}

int32_t HttpEchoServer::GetMaxConnections()
{
	return 1000;
}

bool HttpEchoServer::OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)
{
	//Add Your Code Here
	LOG_INFO(logger, "receive protocol on fd="<<fd<<", content is:\n"<<context->Buffer);

	ProtocolContext *send_context = NewProtocolContext();
	send_context->type = DTYPE_TEXT;
	ByteBuffer *byte_buffer = (ByteBuffer*)send_context;

	HttpRequest *req = (HttpRequest*)context->protocol;
	char c = req->url[req->url_len];
	req->url[req->url_len] = '\0';

	//handle request
	char data[1024];
	if(strcmp(req->url, "/") == 0)
	{
		req->url[req->url_len] = c;
		sprintf(data, "<html><head><title>Http Echo</title></head><body><center><h4>Welcome to EasyNet.</h4></center><br><br>Your request content is:<br><textarea cols=\"100\" rows=\"15\">%s</textarea></p></body><html>", context->Buffer);
		sprintf(byte_buffer->Buffer, "HTTP/1.1 200 OK\r\nContent-Length:%d\r\nContent-Type:text/html\r\n\r\n%s", strlen(data), data);
		byte_buffer->Size = strlen(byte_buffer->Buffer);
	}
	else
	{
		char path[256];
		sprintf(path, "./html/%s", req->url);

		struct stat stat_buf;
		if(stat(path, &stat_buf) == -1)
		{
			sprintf(byte_buffer->Buffer, "HTTP/1.1 404 Not Found\r\n");
			byte_buffer->Size = strlen(byte_buffer->Buffer);

			sprintf(path, "./html/404.html");
			if(stat(path, &stat_buf) == 0)
			{
				FILE *fp = fopen(path, "rb");
				sprintf(byte_buffer->Buffer+byte_buffer->Size, "Content-Length:%d\r\nContent-Type:text/html\r\n\r\n", stat_buf.st_size);
				byte_buffer->Size = strlen(byte_buffer->Buffer);

				byte_buffer->CheckSize(stat_buf.st_size);

				char *temp = byte_buffer->Buffer+byte_buffer->Size;
				uint32_t read_size = fread(temp, 1, stat_buf.st_size, fp);
				byte_buffer->Size += read_size;
			}
		}
		else
		{
			if(S_ISREG(stat_buf.st_mode))
			{
				FILE *fp = fopen(path, "rb");
				if(fp == NULL)
				{
					LOG_ERROR(logger, "open file=%s"<<path<<" failed. errno="<<errno<<"("<<strerror(errno)<<")");
					sprintf(byte_buffer->Buffer, "HTTP/1.1 500 Internal Server error\r\n");
					byte_buffer->Size = strlen(byte_buffer->Buffer);
				}
				else
				{
					sprintf(byte_buffer->Buffer, "HTTP/1.1 200 OK\r\nContent-Length:%d\r\nContent-Type:text/html\r\n\r\n", stat_buf.st_size);
					byte_buffer->Size = strlen(byte_buffer->Buffer);
					byte_buffer->CheckSize(stat_buf.st_size);

					char *temp = byte_buffer->Buffer+byte_buffer->Size;
					uint32_t read_size = fread(temp, 1, stat_buf.st_size, fp);
					byte_buffer->Size += read_size;

					fclose(fp);
				}
			}
		}
	}

	req->url[req->url_len] = c;

	send_context->protocol_type = (uint32_t)req->keep_alive;
	SendProtocol(fd, send_context);

	return true;
}

void HttpEchoServer::OnSendSucc(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_DEBUG(logger, "send protocol succ on fd="<<fd<<", info='"<<context->Info<<"'");
	bool keep_alive = (bool)context->protocol_type;
	if(!keep_alive)
		NotifySocketFinish(fd);
	else
	{
		IEventServer *event_server = GetEventServer();
		IEventHandler *trans_handler = GetTransHandler();
		event_server->SetTimeout(fd, 20000);
	}
	DeleteProtocolContext(context);
	return ;
}

void HttpEchoServer::OnSendError(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_ERROR(logger, "send protocol failed on fd="<<fd<<", info='"<<context->Info<<"'");
	DeleteProtocolContext(context);
	NotifySocketFinish(fd);
	return ;
}

void HttpEchoServer::OnSendTimeout(int32_t fd, ProtocolContext *context)
{
	//Add Your Code Here
	LOG_WARN(logger, "send protocol timeout on fd="<<fd<<", info='"<<context->Info<<"'");
	DeleteProtocolContext(context);
	NotifySocketFinish(fd);
	return ;
}

void HttpEchoServer::OnSocketFinished(int32_t fd)
{
	//Add Your Code Here
	LOG_INFO(logger, "socket finished. close it. fd="<<fd);
	Socket::Close(fd);

	return ;
}

IProtocolFactory* HttpEchoServer::GetProtocolFactory()
{
	return m_ProtocolFactory;
}

