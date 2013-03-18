/*
 * TransHandler.h
 *
 *  Created on: Mar 18, 2013
 *      Author: LiuYongJin
 */

#ifndef _TRANS_HANDLER_H_
#define _TRANS_HANDLER_H_

#include <assert.h>
#include <common/logger.h>
#include <common/eventhandler.h>
#include <common/socket_manager.h>
#include <framework/messagefactory.h>


namespace easynet
{

class TransHandler:public EventHandler
{
public:
	TransHandler(MsgFactory *msg_factory, SocketManager<SocketWapper> *socket_manager)
		:m_msg_factory(msg_factory)
		,m_socket_mgr(socket_manager)
	{
		assert(m_msg_factory != NULL);
		assert(m_socket_mgr != NULL);
	}
public:
	virtual HANDLE_RESULT on_fd_readable(uint32_t fd);
	virtual HANDLE_RESULT on_fd_writeable(uint32_t fd);
	virtual HANDLE_RESULT on_fd_timeout(uint32_t fd);
	virtual HANDLE_RESULT on_fd_error(uint32_t fd);
private:
	MsgFactory *m_msg_factory;
	SocketManager<SocketWapper> *m_socket_mgr;
private:
	DECL_LOGGER(logger);
};

}//namespace
#endif //_TRANS_HANDLER_H_


