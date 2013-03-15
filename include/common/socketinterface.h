/*
 * socketinterface.h
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

namespace easynet
{

class ISocket
{
public:
	ISocket():m_fd(-1) ,m_port(-1) ,m_addr(NULL){}
	ISocket(int32_t fd, char *addr, int32_t port);
	virtual ~ISocket(){}

	bool init(int32_t fd, char *addr, int32_t port);  //对未设置过的socket赋值,成功返回true,失败返回false
	int32_t get_fd(){return m_fd;}
	char* get_addr(){return m_addr;}
	int32_t get_port(){return m_fd;}
	bool is_valid(){return m_fd==-1?false:true;}
	void close();
private:
	int32_t m_fd;
	int32_t m_port;
	char *m_addr;
};

inline
ISocket::ISocket(int32_t fd, char *addr, int32_t port)
	:m_fd(fd)
	,m_port(port)
	,m_addr(NULL)
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
	close();
	if(m_addr != NULL)
		free(m_addr);
	m_addr = NULL;
}

inline
bool ISocket::init(int32_t fd, char *addr, int32_t port)
{
	if(m_fd > 0)
		return false;
	m_fd = fd;
	m_port = port;
	uint32_t len0;
	if(addr!=NULL && (len0=strlen(addr))>0)
	{
		if(m_addr==NULL || strlen(m_addr)<len0)
		{
			if(m_addr != NULL)
				free(m_addr);
			m_addr = (char*)malloc(len0+1);
		}
		memcpy(m_addr, addr, len0+1);
	}
	return true;
}

void ISocket::close()
{
	if(m_fd > 0)
		::close(m_fd);
	m_fd = -1;
	m_port = -1;
}

}//namespace

#endif //_COMMON_SOCKET_INTERFACE_H_


