/*
 * ClientMain.cpp
 *
 *  Created on: 2013-5-18
 *      Author: tim
 */

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

	//写协议头数据
	if(bytebuffer.m_Capacity < protocol_factory.HeaderSize())
		bytebuffer.Enlarge(protocol_factory.HeaderSize());
	//protocol_factory.EncodeHeader(bytebuffer.m_Buffer, 0);    //body_size未知,后面再写
	bytebuffer.m_Size = protocol_factory.HeaderSize();

	KVData kvdata;
	kvdata.AttachWriteBuffer(&bytebuffer, true);
	kvdata.SetValue(Index_ProtocolType, 0);
	kvdata.SetValue(Index_ClientID, 0);
	kvdata.SetValue(Index_ClientString, "I'm a client.");

	uint32_t body_size = bytebuffer.m_Size-protocol_factory.HeaderSize();
	protocol_factory.EncodeHeader(bytebuffer.m_Buffer, body_size);

	if(Socket::SendAll(fd, bytebuffer.m_Buffer, bytebuffer.m_Size) == -1)
	{
		printf("send data failed.\n");
		return -1;
	}

	return 0;
}
