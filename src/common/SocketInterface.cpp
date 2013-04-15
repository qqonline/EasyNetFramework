/*
 * SocketInterface.cpp
 *
 *  Created on: Mar 15, 2013
 *      Author: LiuYongJin
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <common/SocketInterface.h>

namespace easynet
{

IMPL_LOGGER(ISocket, logger);

bool ISocket::init(int32_t fd, char *addr, int32_t port, bool block)
{
	if(m_fd > 0)
		return false;
	m_fd = fd;
	m_port = port;
	m_block = block;

	uint32_t len0;
	if(addr!=NULL && (len0=strlen(addr))>0)
	{
		if(m_addr==NULL || strlen(m_addr)<len0)
		{
			if(m_addr != NULL)
				free(m_addr);
			m_addr = (char*)malloc(len0+1);
			assert(m_addr != NULL);
		}
		memcpy(m_addr, addr, len0+1);
	}
	return true;
}

bool ISocket::set_block(bool block)
{
	if(m_fd<0 || m_block == block)
		return true;
	int32_t flags = fcntl(m_fd, F_GETFL, 0);
	if(flags == -1 )
	{
		LOG4CPLUS_ERROR(logger, "get fd flag error. errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd);
		return false;
	}
	if(!m_block)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if(fcntl(m_fd, F_SETFL, flags) == -1 )
	{
		LOG4CPLUS_ERROR(logger, "set fd flag error. errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd);
		return false;
	}
	m_block = block;
	return true;
}

bool ISocket::create_socket()
{
	//1. 创建socket
	m_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(m_fd < 0)
	{
		LOG4CPLUS_ERROR(logger, "create socket error. errno="<<errno<<"["<<strerror(errno)<<"]");
		return false;
	}

	//2. 设置属性
	int flags = fcntl(m_fd, F_GETFL, 0);
	if(flags == -1)
	{
		LOG4CPLUS_ERROR(logger, "get socket flag error. errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd);
		this->close();
		return false;
	}
	if(!m_block)              //non block mode
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	flags |= FD_CLOEXEC;      //close on exec
	if(fcntl(m_fd, F_SETFL, flags) == -1)
	{
		LOG4CPLUS_ERROR(logger, "set socket flag error. errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd);
		this->close();
		return false;
	}

	return true;
}

bool ISocket::is_ip(const char *addr)
{
	if(addr == NULL)
		return false;
	int d[4];
	int32_t i = sscanf(addr, "%d.%d.%d.%d",&d[0], &d[1], &d[2], &d[3]);
	if(i != 4)
		return false;
	if(d[0]<0||d[0]>255
		||d[1]<0||d[1]>255
		||d[2]<0||d[2]>255
		||d[3]<0||d[3]>255)
		return false;
	for(i=0; addr[i]!='\0'; ++i)
		if(addr[i]!='.' && (addr[i]<'0'||addr[i]>'9'))
			return false;
	return true;
}

}//namespace
