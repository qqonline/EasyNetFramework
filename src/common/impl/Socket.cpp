/*
 * Socket.cpp
 *
 *  Created on: Mar 15, 2013
 *      Author: LiuYongJin
 */

#include <fcntl.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <errno.h>

#include "Socket.h"

namespace easynet
{

bool Socket::IsIp(const char *addr)
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
	for(i=0; addr[i]!='\0'; ++i)  //判断是否有其他非法字符
		if(addr[i]!='.' && (addr[i]<'0'||addr[i]>'9'))
			return false;
	return true;
}

int32_t Socket::CreateSocket(bool block/*=true*/, bool close_exec/*=true*/)
{
	int32_t fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
		return -1;
	int flags = fcntl(fd, F_GETFL, 0);
	if(flags == -1)
	{
		close(fd);
		return -1;
	}
	if(block)
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;

	if(close_exec == true)
		flags |= FD_CLOEXEC;      //close on exec
	if(fcntl(fd, F_SETFL, flags) == -1)
	{
		close(fd);
		return -1;
	}
	return fd;
}


int32_t Socket::CreateListenSocket(uint32_t port, const char *ip, bool block, bool reuse, bool close_exec)
{
	int32_t fd = CreateSocket(block, close_exec);
	if(fd == -1)
		return -1;

	if(reuse == true)
	{
		int _reuse = 1;
		if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&_reuse, sizeof(_reuse)) == -1)
		{
			close(fd);
			return -1;
		}
	}

	//绑定到端口
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(ip==NULL ||ip[0]=='\0')
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else if(IsIp(ip))  //ip地址
		addr.sin_addr.s_addr = inet_addr(ip);
	else  //域名
	{
		struct hostent *hent = gethostbyname(ip);
		if(hent==NULL || (hent->h_addrtype!=AF_INET && hent->h_addrtype!=AF_INET6))
		{
			close(fd);
			return -1;
		}
		addr.sin_addr.s_addr = inet_addr(hent->h_addr);
	}

	if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		close(fd);
		return -1;
	}

	return fd;
}

int32_t Socket::Connect(uint32_t port, const char *ip, bool block/*=true*/, int32_t wait_ms/*=-1*/)
{
	int32_t fd;
	if((fd=CreateSocket(block)) == -1)
		return -1;

	//连接到服务器
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(IsIp(ip))
		addr.sin_addr.s_addr = inet_addr(ip);
	else
	{
		struct hostent *hent = gethostbyname(ip);
		if(hent==NULL || (hent->h_addrtype!=AF_INET && hent->h_addrtype!=AF_INET6))
		{
			close(fd);
			return -1;
		}
		addr.sin_addr.s_addr = inet_addr(hent->h_addr);
	}

	if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != -1)    //成功直接返回
		return fd;
	if(block || (errno!=EINPROGRESS&&errno!=EWOULDBLOCK&&errno!=EINTR))
	{
		close(fd);
		return -1;
	}

	if(wait_ms == 0)    //不需要等直接返回
		return fd;

	//非阻塞并且等待建立连接
	struct timeval tval;
	fd_set rset, wset;
	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	FD_ZERO(&wset);
	FD_SET(fd, &wset);

	tval.tv_sec  = wait_ms/1000;
	tval.tv_usec = (wait_ms%1000)*1000;

	int32_t tmp = select(fd+1, (fd_set*)&rset, (fd_set*)&wset, (fd_set*)NULL, &tval);
	if (tmp <= 0)
	{
		close(fd);
		return -1;
	}

	if(FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset))
	{
		int error;
		int len = sizeof(error);
		tmp = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&error, (socklen_t*)&len);
		if(tmp<0 || (tmp==0&&error!=0))
		{
			close(fd);
			return -1;
		}
	}

	return fd;
}

int32_t Socket::RecvAll(int32_t fd, char *buffer, uint32_t read_size)
{
	int32_t total_recv = 0;
	while(total_recv < read_size)
	{
		int32_t temp = recv(fd, buffer+total_recv, read_size-total_recv, 0);
		if(temp==0 || (temp==-1&&errno!=EAGAIN&&errno!=EINTR&&errno!=EWOULDBLOCK))
			return temp;
		total_recv += temp;
	}
	return total_recv;
}

int32_t Socket::SendAll(int32_t fd, char *buffer, uint32_t send_size)
{
	int32_t total_send = 0;
	while(total_send < send_size)
	{
		int32_t temp = send(fd, buffer+total_send, send_size-total_send, 0);
		if(temp==-1 && errno!=EAGAIN && errno!=EINTR && errno!=EWOULDBLOCK)
			return temp;
		total_send += temp;
	}
	return total_send;
}

bool Socket::SetBlock(int32_t fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if(flags == -1)
		return false;
	flags &= ~O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flags) == -1)
		return false;
	return true;
}

bool Socket::SetNoBlock(int32_t fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if(flags == -1)
		return false;
	flags |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flags) == -1)
		return false;
	return true;
}

}//namespace
