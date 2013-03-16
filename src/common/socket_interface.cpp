/*
 * socketinterface.cpp
 *
 *  Created on: Mar 15, 2013
 *      Author: LiuYongJin
 */
#include <fcntl.h>
#include <errno.h>

#include <common/socketinterface.h>

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

}//namespace
