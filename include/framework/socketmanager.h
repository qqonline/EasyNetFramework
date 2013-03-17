/*
 * socketmanager.h
 *
 *  Created on: 2013-3-17
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_SOCKET_MANAGER_H_
#define _FRAMEWORK_SOCKET_MANAGER_H_

#include <stdint.h>

#include <common/objectpool.h>
#include <framework/socketwapper.h>

namespace easynet
{

class SocketManager
{
public:
	//n_cache:socketpool中最多保留的空闲socket对象个数
	SocketManager(uint32_t n_cache=512):m_socket_pool(n_cache){}
	virtual ~SocketManager();


private:
	ObjectPool m_socket_pool;
};


}//namespace
#endif //_FRAMEWORK_SOCKET_MANAGER_H_


