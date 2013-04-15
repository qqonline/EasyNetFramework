/*
 * SocketManager.h
 *
 *  Created on: Mar 18, 2013
 *      Author: LiuYongJin
 */


#ifndef _COMMON_SOCKET_MANAGER_H_
#define _COMMON_SOCKET_MANAGER_H_
#include <stdint.h>
#include <string>
#include <assert.h>
#include <map>
using std::string;
using std::map;

#include <new>

#include <common/SocketTransceiver.h>
#include <common/ObjectPool.h>
#include <common/Logger.h>

namespace easynet
{

template <class SocketType>
class SocketManager
{
public:
	SocketManager(string addr, uint32_t port, bool block=false, uint32_t n_cache=OBPOOL_MAX_FREE)
		:m_socket_pool(sizeof(SocketType), n_cache)
		,m_addr(addr)
		,m_port(port)
		,m_block(block)
	{}
	virtual ~SocketManager(){}

	SocketType* get_active(int32_t wait_ms=0);
	SocketType* get_passive(int32_t fd, const char *peer_addr, uint32_t peer_port);
	SocketType* find(int32_t fd);
	bool release(SocketType *socket);
	uint32_t size(){return m_socket_map.size();}
private:
	ObjectPool m_socket_pool;
	string m_addr;
	uint32_t m_port;
	bool m_block;

	map<int32_t, SocketType*> m_socket_map;
private:
	DECL_LOGGER(logger);
};

template<class SocketType>
IMPL_LOGGER(SocketManager<SocketType>, logger);

template <class SocketType>
SocketType* SocketManager<SocketType>::get_active(int32_t wait_ms/*=0*/)
{
	SocketType *socket = (SocketType*)m_socket_pool.get();
	if(socket == NULL)
		return NULL;
	socket = new((void*)socket) SocketType(-1, m_addr.c_str(), m_port, m_block, true);
	if(!socket->open(wait_ms))
	{
		m_socket_pool.recycle(socket);
		return NULL;
	}
	int32_t fd = socket->get_fd();
	if(m_socket_map.find(fd) != m_socket_map.edn())
	{
		LOG4CPLUS_WARN(logger, "create active socket success, but find fd="<<fd<<" in socket_map. remove it.");
		m_socket_map.erase(fd);
	}
	m_socket_map.insert(std::make_pair(fd, socket));

	return socket;
}

template <class SocketType>
SocketType* SocketManager<SocketType>::get_passive(int32_t fd, const char *peer_addr, uint32_t peer_port)
{
	SocketType *socket = (SocketType*)m_socket_pool.get();
	if(socket == NULL)
		return NULL;
	if(m_socket_map.find(fd) != m_socket_map.end())
	{
		LOG4CPLUS_WARN(logger, "find fd="<<fd<<" in socket_map when create passive socket. remove it.");
		m_socket_map.erase(fd);
	}
	socket = new((void*)socket) SocketType(fd, peer_addr, peer_port, m_block, false);
	bool ret = socket->open(0);
	assert(ret == true);
	m_socket_map.insert(std::make_pair(fd, socket));

	return socket;
}

template <class SocketType>
bool SocketManager<SocketType>::release(SocketType *socket)
{
	assert(socket != NULL);
	map<int32_t, SocketType*>::iterator it = m_socket_map.find(socket->get_fd());
	if(it == m_socket_map.end())
		return false;
	socket->~SocketType();
	m_socket_map.erase(it);
	m_socket_pool.recycle((void*)socket);
	return true;
}

template <class SocketType>
SocketType* SocketManager<SocketType>::find(int32_t fd)
{
	map<int32_t, SocketType*>::iterator it = m_socket_map.find(fd);
	return it==m_socket_map.end()?NULL:it->second;
}



}//namespace

#endif //_COMMON_SOCKET_MANAGER_H_

