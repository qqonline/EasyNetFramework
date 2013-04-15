/*
 * SocketTransceiver.h
 *
 *  Created on: Mar 15, 2013
 *      Author: LiuYongJin
 */

#ifndef _COMMON_SOCKET_TRANS_H_
#define _COMMON_SOCKET_TRANS_H_

#include <stdint.h>

#include <common/SocketInterface.h>
#include <common/Logger.h>

namespace easynet
{

//用于传输数据的socket
class SocketTrans:public ISocket
{
public:
	SocketTrans(bool active)    //active:true主动链接;false被动链接
		:m_active(active)
	{}
	SocketTrans(int32_t fd, char *addr, int32_t port, bool block, bool active)
		:m_active(active)
		,ISocket(fd, addr, port, block)
	{}

	virtual ~SocketTrans(){}
	bool open(int32_t wait_ms);    //基类接口

	bool is_active(){return m_active;}
	//尽可能多的发送数据.返回发送的数据大小,失败返回-1
	int32_t send(char *data, uint32_t size);
	//发送全部数据
	bool send_all(char *data, uint32_t size);

	//尽可能多的接收数据.返回接收的大小,失败返回-1
	int32_t receive(char *data, uint32_t size);
	//接收全部数据
	bool receive_all(char *data, uint32_t size);
private:
	bool m_active;
private:
	DECL_LOGGER(logger);
};

}//namespace

#endif //_COMMON_SOCKET_TRANS_H_


