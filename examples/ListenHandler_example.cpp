/*
 * ListenHandler_example.cpp
 *
 *  Created on: 2013-5-31
 *      Author: tim
 */
#include "Socket.h"
#include "EventServerEpoll.h"
#include "ListenHandler.h"
using namespace easynet;

int main()
{
	EventServerEpoll *event_server = new EventServerEpoll(1024);

	ListenHandler *listen_handler = new ListenHandler((IAppInterface*)10);
	int32_t fd = Socket::CreateListenSocket(12300);
	Socket::Listen(fd);
	listen_handler->OnEventRead(fd, 1234L);

	event_server->AddEvent(fd, ET_PER_RD, listen_handler, -1);
	event_server->RunLoop();

	return 0;
}
