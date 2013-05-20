/*
 * ClientMain.cpp
 *
 *  Created on: 2013-5-18
 *      Author: tim
 */

#include "Socket.h"
#include "KVDataProtocolFactory.h"

//协议定义
#include "KVDataIndex.h"
#include "ProtocolDefine.h"

using namespace easynet;
int main()
{
	uint32_t port = 10000;
	char *server_ip = "127.0.0.1";
	int32_t fd = Socket::Connect(port, server_ip);
	if(fd <= 0)
	{
		printf("connect server failed.port=%d, addr=%s\n", port, server_ip);
		return -1;
	}

	KVDataProtocolFactory protocol_factory;
	ProtocolContext *context = protocol_factory.NewSendContext();
	KVData *kv_data = (KVData*)context->protocol;

	//client info
	kv_data->Set(IndexProtocolType, PROTOCOL_CLIENT_INFO);
	char *str = "I'm a client.";
	uint32_t str_len = strlen(str)+1;
	kv_data->Set(IndexData, str, str_len); //包括'\0'

	protocol_factory.EncodeProtocol(context);
	Socket::SendAll(fd, context->buffer, context->cur_data_size);
	return 0;
}
