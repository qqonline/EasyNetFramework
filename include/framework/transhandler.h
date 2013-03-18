/*
 * TransHandler.h
 *
 *  Created on: Mar 18, 2013
 *      Author: LiuYongJin
 */

#ifndef _TRANS_HANDLER_H_
#define _TRANS_HANDLER_H_

#include <common/eventhandler.h>

namespace easynet
{

class TransHandler:public EventHandler
{
public:
	virtual HANDLE_RESULT on_fd_readable(uint32_t fd);
	virtual HANDLE_RESULT on_fd_writeable(uint32_t fd);
	virtual HANDLE_RESULT on_fd_timeout(uint32_t fd);
	virtual HANDLE_RESULT on_fd_error(uint32_t fd);
};

}//namespace
#endif //_TRANS_HANDLER_H_


