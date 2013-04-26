/*
 * SocketTransciver.cpp
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

#include "SocketTransceiver.h"

namespace easynet
{

IMPL_LOGGER(SocketTrans, logger);

bool SocketTrans::Open(int32_t wait_ms)
{
	if(!m_active)    //被动连接
	{
		if(IsValid())
			return true;
		LOG4CPLUS_ERROR(logger, "is a no-active socket and fd is invalid.");
		return false;
	}
	if(IsValid())    //主动连接
	{
		LOG4CPLUS_DEBUG(logger, "active socket has already opened. fd="<<m_port<<" addr="<<m_addr<<":"<<m_port);
		return true;
	}

	if(m_port<=0 || m_addr[0]=='\0')
	{
		LOG4CPLUS_ERROR(logger, "port or addr of active socket is invalid.");
		return false;
	}

	if(!_CreateSocket())
		return false;

	//连接到服务器
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	if(IsIP(m_addr))
		addr.sin_addr.s_addr = inet_addr(m_addr);
	else
	{
		struct hostent *hent = gethostbyname(m_addr);
		if(hent==NULL || (hent->h_addrtype!=AF_INET && hent->h_addrtype!=AF_INET6))
		{
			LOG4CPLUS_ERROR(logger, "get ip by hostname error. fd="<<m_fd<<" addr:"<<m_addr<<" errno="<<errno<<"("<<strerror(errno)<<")" );
			Close();
			return false;
		}
		addr.sin_addr.s_addr = inet_addr(hent->h_addr);
	}

	if(connect(m_fd, (struct sockaddr*)&addr, sizeof(addr)) != -1)
		return true;
	if(m_block || errno!=EINPROGRESS)
	{
		LOG4CPLUS_ERROR(logger, "connect to server error. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
		Close();
		return false;
	}

	if(wait_ms == 0)    //不需要等直接返回true
	{
		LOG4CPLUS_WARN(logger, "connect to server INPROGRESS. no to wait.");
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
		LOG4CPLUS_ERROR(logger, "select error when connecting server. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
		Close();
		return false;
	}

	if(FD_ISSET(m_fd, &rset) || FD_ISSET(m_fd, &wset))
	{
		int error;
		int len = sizeof(error);
		tmp = getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (void*)&error, (socklen_t*)&len);
		if(tmp<0 || (tmp==0&&error!=0))
		{
			LOG4CPLUS_ERROR(logger, "error when connecting server. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
			Close();
			return false;
		}
	}

	return true;
}

int32_t SocketTrans::Send(char *data, uint32_t size)
{
	if(data==NULL || size==0)
		return 0;
	int32_t send_bytes = send(m_fd, data, size, 0);
	if(send_bytes >= 0)//非阻塞时可能只发送部分数据
		return send_bytes;
	if(errno==EWOULDBLOCK || errno==EINTR || errno==EAGAIN)
	{
		LOG4CPLUS_INFO(logger, "send data block. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
		return 0;
	}
	LOG4CPLUS_ERROR(logger, "send data error. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
	return -1;
}

bool SocketTrans::SendAll(char *data, uint32_t size)
{
	if(data==NULL || size==0)
		return true;

	int32_t ret = 0;
	uint32_t send_bytes = 0;
	uint32_t last_size = 0;
	while(send_bytes < size)
	{
		last_size = size-send_bytes;
		ret = send(m_fd, data+send_bytes, last_size, 0);
		if(ret>=0)
		{
			send_bytes += ret;
			continue;
		}
		else if(errno==EWOULDBLOCK || errno==EINTR || errno==EAGAIN)
		{
			LOG4CPLUS_DEBUG(logger, "send data would_block. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
			continue;
		}
		LOG4CPLUS_ERROR(logger, "send data error. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
		return false;
	}

	return true;
}

int32_t SocketTrans::Receive(char *data, uint32_t size)
{
	if(data==NULL || size==0)
	{
		LOG4CPLUS_ERROR(logger, "receive parameter error. fd="<<m_fd);
		return -1;
	}
	int32_t recv_bytes = recv(m_fd, data, size, 0);
	if(recv_bytes > 0)
		return recv_bytes;
	if(recv_bytes == 0)
	{
		LOG4CPLUS_DEBUG(logger, "peer close socket gracefully. fd="<<m_fd);
		return -1;
	}
	if(errno==EAGAIN || errno==EINTR || errno==EWOULDBLOCK)
	{
		LOG4CPLUS_DEBUG(logger, "receive data would_block. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
		return 0;
	}
	LOG4CPLUS_ERROR(logger, "receive data error. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
	return -1;
}

bool SocketTrans::ReceiveAll(char *data, uint32_t size)
{
	if(data==NULL || size==0)
	{
		LOG4CPLUS_ERROR(logger, "receive parameter error. fd="<<m_fd);
		return false;
	}

	int ret = 0;
	int recv_bytes = 0; //已读数据大小
	int last_bytes = 0; //剩下未读数据大小
	while(recv_bytes < size)
	{
		last_bytes = size-recv_bytes;
		ret = recv(m_fd, data+recv_bytes, last_bytes, 0);
		if(ret > 0)
		{
			recv_bytes += ret;
			continue;
		}
		else if(ret == 0)
		{
			LOG4CPLUS_DEBUG(logger, "peer close socket gracefully. fd="<<m_fd);
			return false;
		}
		else if(errno==EAGAIN || errno==EINTR || errno==EWOULDBLOCK)
		{
			LOG4CPLUS_DEBUG(logger, "receive data would_block. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
			continue;
		}
		LOG4CPLUS_ERROR(logger, "receive data error. fd="<<m_fd<<" errno="<<errno<<"("<<strerror(errno)<<")");
		return false;
	}

	return true;
}


}//namespace


