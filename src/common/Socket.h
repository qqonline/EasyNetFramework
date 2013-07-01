/*
 * Socket.h
 *
 *  Created on: 2013-3-15
 *      Author: LiuYongJin
 */

#ifndef _COMMON_SOCKET_H_
#define _COMMON_SOCKET_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

namespace easynet
{

class Socket
{
public:
	//判断addr是否是一个有效的ip地址(点分开的十进制ip地址)
	static bool IsIp(const char *addr);

	//创建socket,失败返回-1,成功返回socket fd.
	//  @param block      : false阻塞,true非阻塞
	//  @param close_exec : exec时是否关闭
	static int32_t CreateSocket(bool block=true, bool close_exec=true);
	//关闭socket
	static void Close(int32_t fd);

	//创建用于监听的socket
	//  @param port       : 需要监听的端口
	//  @param block      : 是否阻塞
	//  @param reuse      : 端口是否可以重用
	//  @param close_exec : 执行exec的时候是否关闭
	//  @param ip         : 指定本地监听的ip(没有指定使用默认的INADDR_ANY)
	//  return            : 成功创建的socket fd, 失败返回-1
	static int32_t CreateListenSocket(uint32_t port, const char *ip="", bool block=true, bool reuse=true, bool close_exec=true);
	static bool Listen(int32_t fd, uint32_t back_log=128);

	//连接服务
	//  @param port       : 需要连接的服务端口
	//  @param ip         : 需要连接的服务器地址(支持域名)
	//  @param block      : 是否阻塞
	//  @wait_time        : 等待连接的时间. -1:一直等待直到连接成功或失败, 0:不等待直接返回, 大于0:等待的毫秒数
	//  @return           : socket fd, 失败返回-1
	static int32_t Connect(uint32_t port, const char *ip, bool block=true, int32_t wait_ms=-1);

	static int32_t Recv(int32_t fd, char *buffer, uint32_t read_size);
	static int32_t RecvAll(int32_t fd, char *buffer, uint32_t read_size);

	static int32_t Send(int32_t fd, char *buffer, uint32_t send_size);
	static int32_t SendAll(int32_t fd, char *buffer, uint32_t send_size);
};

inline
void Socket::Close(int32_t fd)
{
	close(fd);
}

inline
bool Socket::Listen(int32_t fd, uint32_t back_log/*=128*/)
{
	return listen(fd, back_log)==-1?false:true;
}

inline
int32_t Socket::Recv(int32_t fd, char *buffer, uint32_t read_size)
{
	return recv(fd, buffer, read_size, 0);
}

inline
int32_t Socket::Send(int32_t fd, char *buffer, uint32_t send_size)
{
	return send(fd, buffer, send_size, 0);
}


}//namespace

#endif //_COMMON_SOCKET_INTERFACE_H_


