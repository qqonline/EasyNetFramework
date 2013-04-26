/*
 * SocketInterface.h
 *
 *  Created on: 2013-3-15
 *      Author: LiuYongJin
 */

#ifndef _COMMON_SOCKET_INTERFACE_H_
#define _COMMON_SOCKET_INTERFACE_H_

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
	static bool is_ip(const char *addr);
public:
	ISocket():m_fd(-1), m_port(-1), m_addr(NULL), m_block(true){}
	ISocket(int32_t fd, char *addr, int32_t port, bool block);
	virtual ~ISocket();

	bool init(int32_t fd, char *addr, int32_t port, bool block);  //对未设置过的socket赋值,成功返回true,失败返回false
	int32_t get_fd(){return m_fd;}
	char* get_addr(){return m_addr;}
	int32_t get_port(){return m_fd;}
	bool is_block(){return m_block;}
	bool is_valid(){return m_fd<=0?false:true;}
	bool set_block(bool block);    //block:true设置为阻塞模式;false设置为非阻塞模式;成功返回true,失败返回false

	void close();
	//打开socket:
	//wait_ms:socket打开等待时间(单位ms)
	//成功返回true,失败返回false
	virtual bool open(int32_t wait_ms)=0;
protected:
	int32_t m_fd;
	int32_t m_port;
	char *m_addr;
	bool m_block;

	virtual bool create_socket();
private:
	DECL_LOGGER(logger);
};

inline
ISocket::ISocket(int32_t fd, char *addr, int32_t port, bool block)
	:m_fd(fd)
	,m_port(port)
	,m_addr(NULL)
	,m_block(block)
{
	if(addr != NULL)
	{
		uint32_t len = strlen(addr)+1;
		m_addr = (char*)malloc(len);
		memcpy(m_addr, addr, len);
	}
}

inline
ISocket::~ISocket()
{
	this->close();
	if(m_addr != NULL)
		free(m_addr);
	m_addr = NULL;
}

inline
void ISocket::close()
{
	if(m_fd > 0)
	{
		LOG4CPLUS_DEBUG(logger, "close fd="<<m_fd);
		::close(m_fd);
	}
	m_fd = -1;
	m_port = -1;
}


}//namespace

#endif //_COMMON_SOCKET_INTERFACE_H_


