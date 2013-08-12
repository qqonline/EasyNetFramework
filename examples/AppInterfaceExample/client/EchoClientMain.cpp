/*
 * ClientMain.cpp
 *
 *  Created on: 2013-5-18
 *      Author: tim
 */
#include <stdio.h>
#include "Socket.h"
#include "KVDataProtocolFactory.h"
using namespace easynet;

#include "KVDataIndex.h"

int main()
{
	uint32_t port = 12300;
	const char *server_ip = "127.0.0.1";
	int32_t fd = Socket::Connect(port, server_ip);
	if(fd <= 0)
	{
		printf("connect server failed.port=%d, addr=%s\n", port, server_ip);
		return -1;
	}

	KVDataProtocolFactory protocol_factory;
	ByteBuffer bytebuffer;
	KVData kvdata(true);

	//写协议头数据
	if(bytebuffer.Capacity < protocol_factory.HeaderSize())
		bytebuffer.Enlarge(protocol_factory.HeaderSize());
	//预留头部
	bytebuffer.Size = protocol_factory.HeaderSize();

	int32_t protocol_client = 0;
	int32_t client_id = 0;
	const char* client_string = "I'm a Client.";

	kvdata.SetValue(Index_ProtocolType, protocol_client);
	kvdata.SetValue(Index_ClientID, client_id);
	kvdata.SetValue(Index_ClientString, client_string);
	uint32_t body_size = kvdata.Serialize(bytebuffer.Buffer+bytebuffer.Size);
	bytebuffer.Size += body_size;

	protocol_factory.EncodeHeader(bytebuffer.Buffer, body_size);

	if(Socket::SendAll(fd, bytebuffer.Buffer, bytebuffer.Size) == -1)
	{
		printf("send data failed.\n");
		return -1;
	}

	//清空数据
	bytebuffer.Size = 0;
	//接收协议头数据
	uint32_t header_size = protocol_factory.HeaderSize();
	if(bytebuffer.Capacity < header_size)
		bytebuffer.Enlarge(header_size);
	uint32_t recv_size = Socket::RecvAll(fd, bytebuffer.Buffer, header_size);
	if(recv_size != header_size)
	{
		printf("receive header failed. header_size=%d, recv_size=%d\n",header_size, recv_size);
		return -1;
	}

	//解码协议头
	DataType type = DTYPE_INVALID;
	body_size = 0;
	if(protocol_factory.DecodeHeader(bytebuffer.Buffer, type, body_size) == DECODE_ERROR)
	{
		printf("decode header failed.\n");
		return -1;
	}

	//接送协议体
	if(bytebuffer.Capacity < header_size+body_size)
		bytebuffer.Enlarge(body_size);
	recv_size = Socket::RecvAll(fd, bytebuffer.Buffer+header_size, body_size);
	if(recv_size != body_size)
	{
		printf("receive body failed. body_size=%d, recv_size=%d\n", body_size, recv_size);
		return -1;
	}

	//解码协议体
	if(kvdata.UnSerialize(bytebuffer.Buffer+header_size, body_size) == false)
	{
		printf("KVData UnSerialize failed.\n");
		return -1;
	}

	int32_t protocol_server;
	kvdata.GetValue(Index_ProtocolType, protocol_server);
	if(protocol_server == 1)
	{
		int32_t server_id = -1;
		char* server_string = 0;
		uint32_t size = 0;
		kvdata.GetValue(Index_ServerID, server_id);
		kvdata.GetValue(Index_ServerString, server_string, size);
		printf("server_id=%d, server_string=%s\n", server_id, server_string);
	}
	else
	{
		printf("unknow protocol type. %d\n", protocol_server);
		return -1;
	}

	return 0;
}
