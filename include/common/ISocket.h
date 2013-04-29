/*
 * ISocket.h
 *
 *  Created on: 2013-3-15
 *      Author: LiuYongJin
 */

#ifndef _COMMON_ISOCKET_H_
#define _COMMON_ISOCKET_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Logger.h"

namespace easynet
{

class ISocket
{
public:
	//判断给定的addr是不是ip地址
	static bool IsIP(const char *addr);
public:
	ISocket();
	ISocket(int32_t fd, char *addr, int32_t port, bool block);
	virtual ~ISocket();

	//对未设置过的socket赋值,成功返回true,失败返回false
	bool Init(int32_t fd, char *addr, int32_t port, bool block);
	int32_t GetFd(){return m_fd;}
	char* GetAddr(){return m_addr;}
	int32_t GetPort(){return m_port;}
	bool IsBlock(){return m_block;}
	bool IsValid(){return m_fd<=0?false:true;}
	//block:true设置为阻塞模式;false设置为非阻塞模式;成功返回true,失败返回false
	bool SetBlock(bool block);
	void Close();

	/** 打开socket:
	 * @param wait_ms : socket打开等待时间(单位ms),小于0一直等待,0直接返回,大于0等待多时间
	 * @return        : 成功返回true,失败返回false
	 */
	virtual bool Open(int32_t wait_ms)=0;
protected:
	int32_t  m_fd;
	int32_t  m_port;
	char     m_addr[20];
	bool     m_block;

	virtual bool _CreateSocket();
private:
	void _copy_addr(char *addr);
private:
	DECL_LOGGER(logger);
};

inline
ISocket::ISocket()
	:m_fd(-1)
	,m_port(-1)
	,m_block(true)
{
	m_addr[0] = '\0';
}

inline
ISocket::ISocket(int32_t fd, char *addr, int32_t port, bool block)
	:m_fd(fd)
	,m_port(port)
	,m_block(block)
{
	m_addr[0] = '\0';
	if(addr!=NULL)
	{
		assert(strlen(addr)<sizeof(m_addr));
		strcpy(m_addr, addr);
	}
}

inline
ISocket::~ISocket()
{
	Close();
}

inline
void ISocket::Close()
{
	if(m_fd > 0)
		close(m_fd);
	m_fd = -1;
	m_port = -1;
	m_addr[0] = '\0';
}


}//namespace

#endif //_COMMON_SOCKET_INTERFACE_H_


