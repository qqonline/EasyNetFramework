/*
 * SocketListener.cpp
 *
 *  Created on: Mar 15, 2013
 *      Author: LiuYongJin
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <common/SocketListener.h>

namespace easynet
{

IMPL_LOGGER(SocketListener, logger);

bool SocketListener::open(int32_t /*wait_ms*/)
{
	if(is_valid())
	{
		LOG4CPLUS_DEBUG(logger, "socket already opened. fd="<<m_fd);
		return true;
	}
	else if(m_port<=0)
	{
		LOG4CPLUS_DEBUG(logger, "socket port is invalid. port="<<m_port);
		return false;
	}

	if(!create_socket())
		return false;

	//set reuse
	int reuse = 1;
	if(setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuse, sizeof(reuse)) == -1)
	{
		LOG4CPLUS_ERROR(logger, "set socket opt error. errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd);
		this->close();
		return false;
	}

	//绑定到端口
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	if(m_addr == NULL)
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else if(is_ip(m_addr))  //ip地址
		addr.sin_addr.s_addr = inet_addr(m_addr);
	else  //域名
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

	if(bind(m_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		LOG4CPLUS_ERROR(logger, "bind socket error. errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd);
		this->close();
		return false;
	}

	//4. 监听端口.监听队列中等待accept的最大连接数设置为默认值
	if(listen(m_fd, m_backlog) == -1)
	{
		LOG4CPLUS_ERROR(logger, "listen socket error. errno="<<errno<<"["<<strerror(errno)<<"] fd="<<m_fd);
		this->close();
		return false;
	}

	return true;
}

}//namespace


