/*
 * application.h
 *
 *  Created on: 2013-3-17
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_APPLICATION_H_
#define _FRAMEWORK_APPLICATION_H_

#include <common/thread.h>
#include <common/iodemuxer.h>

#include <framework/listenhandler.h>

namespace easynet
{

class Application:public IAcceptor, public Thread
{
public:
	void do_run();
	bool on_new_connect(uint32_t fd, struct sockaddr_in *addr);

public:
	Application();
	~Application();

	bool start();
	bool stop();
private:
	IODemuxer *m_iodemuxer;
};

}//namespace
#endif //_FRAMEWORK_APPLICATION_H_
