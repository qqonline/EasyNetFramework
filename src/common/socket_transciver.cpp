/*
 * socket_transciver.cpp
 *
 *  Created on: Mar 16, 2013
 *      Author: LiuYongJin
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <common/socket_transceiver.h>

namespace easynet
{

IMPL_LOGGER(SocketTrans, logger);

bool SocketTrans::open(int32_t wait_ms)
{
	if(!m_active)    //被动连接
	{
		if(is_valid())
			return true;
		LOG4CPLUS_ERROR(logger, "is a no-active socket and fd is invalid.");
		return false;
	}
	if(is_valid())    //主动连接
	{
		LOG4CPLUS_DEBUG(logger, "active socket has already opened. fd="<<m_port<<" addr="<<m_addr<<":"<<m_port);
		return true;
	}

	if(m_port<=0 || m_addr==NULL || strlen(m_addr)<=0)
	{
		LOG4CPLUS_ERROR(logger, "port or addr of active socket are invalid.");
		return false;
	}

	if(!create_socket())
		return false;

	//连接到服务器
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	if(is_ip(m_addr))
		addr.sin_addr.s_addr = inet_addr(m_addr);
	else
	{
		struct hostent *hent = gethostbyname(m_addr);
		if(hent==NULL || (hent->h_addrtype!=AF_INET && hent->h_addrtype!=AF_INET6))
		{
			LOG4CPLUS_ERROR(logger, "get ip by hostname error. errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd<<" addr:"<<m_addr);
			this->close();
			return false;
		}
		addr.sin_addr.s_addr = inet_addr(hent->h_addr);
	}

	if(connect(m_fd, (struct sockaddr*)&addr, sizeof(addr)) != -1)
		return true;
	if(m_block || !(errno==EINPROGRESS||errno==EINTR))
	{
		LOG4CPLUS_ERROR(logger, "connect to server error. errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd);
		this->close();
		return false;
	}

	if(wait_ms == 0)    //不需要等直接返回true
	{
		LOG4CPLUS_WARN(logger, "connect to server INPROGRESS or EINTR. no to wait.");
		return true;
	}

	//非阻塞并且等待建立连接
	struct timeval tval;
	fd_set rset, wset;
	FD_ZERO(&rset);
	FD_SET(m_fd, &rset);
	FD_ZERO(&wset);
	FD_SET(m_fd, &wset);

	tval.tv_sec  = wait_ms/1000;
	tval.tv_usec = (wait_ms%1000)*1000;

	int32_t tmp = select(m_fd+1, (fd_set*)&rset, (fd_set*)&wset, (fd_set*)NULL, &tval);
	if (tmp <= 0)
	{
		LOG4CPLUS_ERROR(logger, "select error when connecting server.errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd);
		this->close();
		return false;
	}

	if(FD_ISSET(m_fd, &rset) || FD_ISSET(m_fd, &wset))
	{
		int error;
		int len = sizeof(error);
		tmp = getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (void*)&error, (socklen_t*)&len);
		if(tmp<0 || (tmp==0&&error!=0))
		{
			LOG4CPLUS_ERROR(logger, "error when connecting server. errno="<<error<<"["<<strerror(error)<<"] fd="<<m_fd);
			this->close();
			return false;
		}
	}

	return true;
}

}//namespace


