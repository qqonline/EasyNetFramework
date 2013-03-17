/*
 * Application.cpp
 *
 *  Created on: 2013-3-17
 *      Author: LiuYongJin
 */
#include <common/iodemuxer_epoll.h>

#include <framework/application.h>

namespace easynet
{

Application::Application()
{
	m_iodemuxer = new IODemuxerEpoll();
}

Application::~Application()
{
	delete m_iodemuxer;
}

bool Application::on_new_connect(uint32_t fd, struct sockaddr_in *addr)
{

	return true;
}



}//namespace
