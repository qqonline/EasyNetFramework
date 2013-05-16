/*
 * ListenHandler.cpp
 *
 *  Created on: 2013-5-12
 *      Author: LiuYongJin
 */
#include "ListenHandler.h"

#include "IAppInterface.h"

namespace easynet
{
IMPL_LOGGER(ListenHandler, logger);

bool ListenHandler::OnEventRead(int32_t fd, uint64_t now_time)
{
	while(true)
	{
		int32_t new_fd = accept(fd, NULL, 0);
		if(new_fd == -1)
			break;
		bool ret = m_AppInterface->AcceptNewConnect(new_fd);
		if(ret == false)
		{
			LOG_ERROR(logger, "app_interface accept new connect failed. close it. fd="<<new_fd);
			close(new_fd);
		}
	}
	return true;
}

}//namespace

