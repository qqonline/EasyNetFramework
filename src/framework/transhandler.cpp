/*
 * transhandler.cpp
 *
 *  Created on: 2013-3-18
 *      Author: LiuYongJin
 */

#include <framework/transhandler.h>
namespace easynet
{

IMPL_LOGGER(TransHandler, logger);

HANDLE_RESULT TransHandler::on_fd_readable(uint32_t fd)
{
	LOG4CPLUS_DEBUG(logger, "on_fd_readable, fd="<<fd);
	SocketWapper *socket = m_socket_mgr->find(fd);
	if(socket == NULL)
	{
		LOG4CPLUS_ERROR(logger, "can't find socket for fd="<<fd);
		return HANDLE_ERROR;
	}

	ByteBuffer *bytebuf = socket->get_input_buffer();
	uint32_t wait_size = socket->get_input_wait_size();
	if(bytebuf == NULL)
	{
		bytebuf = new ByteBuffer;
		wait_size = m_msg_factory->msg_decode_length();
	}
	assert(bytebuf!=NULL && wait_size>0);

	char *buf = bytebuf->get_append_buffer(wait_size);
	int32_t read_size = socket->receive(buf, wait_size);
	if(read_size < 0)  //出错
	{

	}
	return HANDLE_FINISH;
}
HANDLE_RESULT TransHandler::on_fd_writeable(uint32_t fd)
{

}
HANDLE_RESULT TransHandler::on_fd_timeout(uint32_t fd)
{

}
HANDLE_RESULT TransHandler::on_fd_error(uint32_t fd)
{

}

}//namespace
