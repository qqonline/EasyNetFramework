/*
 * socket_transceiver.h
 *
 *  Created on: Mar 15, 2013
 *      Author: LiuYongJin
 */

#ifndef _COMMON_SOCKET_TRANS_H_
#define _COMMON_SOCKET_TRANS_H_

#include <stdint.h>

#include <common/socket_interface.h>
#include <common/logger.h>

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

	bool is_active(){return m_active;}
	bool open(int32_t wait_ms);
private:
	bool m_active;
private:
	DECL_LOGGER(logger);
};

}//namespace

#endif //_COMMON_SOCKET_TRANS_H_


