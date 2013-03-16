/*
 * ListenHandler.cpp
 *
 *  Created on: 2013-3-16
 *      Author: LiuYongJin
 */

#include <framework/listenhandler.h>

namespace easynet
{

IMPL_LOGGER(ListenHandler, logger);

HANDLE_RESULT ListenHandler::on_fd_readable(uint32_t fd)
{
	while(1)
	{
		struct sockaddr_in addr;
		uint32_t size = sizeof(addr);
		int32_t new_fd = ::accept(fd, (struct sockaddr*)&addr, &size);
		if(new_fd == -1)
		{
			if(!(errno==EAGAIN || errno==EINPROGRESS || errno==EINTR))
				LOG4CPLUS_ERROR(logger, "accept new connect error. errno="<<errno<<"["<<strerror(errno)<<"]");
			break;
		}

		if(!m_acceptor->on_new_connect(new_fd, &addr))
		{
			LOG4CPLUS_ERROR(logger, "acceptor accept new connect failed and close it. fd="<<new_fd);
			::close(new_fd);
		}
	}
	return HANDLE_CONTINUE;
}


}//namespace

