/*
 * listenhandler.h
 *
 *  Created on: 2013-3-16
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_LISTENHANDLER_H_
#define _FRAMEWORK_LISTENHANDLER_H_

#include <stdint.h>
#include <errno.h>
#include <string.h>

#include <common/eventhandler.h>
#include <common/logger.h>

namespace easynet
{

class IAcceptor
{
public:
	virtual ~IAcceptor(){}
	bool on_new_connect(uint32_t fd, const struct sockaddr_in *addr)=0;
};

class ListenHandler: public EventHandler
{
public:
	ListenHandler(IAcceptor *acceptor):m_acceptor(acceptor){}
	virtual ~ListenHandler(){}

	HANDLE_RESULT on_fd_readable(uint32_t fd);
	HANDLE_RESULT on_fd_writeable(uint32_t fd){return HANDLE_CONTINUE;}
	HANDLE_RESULT on_fd_timeout(uint32_t fd){return HANDLE_FINISH;}
private:
	IAcceptor *m_acceptor;
private:
	DECL_LOGGER(logger);
};


}//namespace

#endif //_FRAMEWORK_LISTENHANDLER_H_


