/*
 * iodemuxer_example.cpp
 *
 *  Created on: Mar 14, 2013
 *      Author: LiuYongJin
 */

#include <common/iodemuxer_epoll.h>
#include <common/logger.h>
#include <common/eventhandler.h>

class MyTimer: public TimerHandler
{
public:
	HANDLE_RESULT on_timer_timeout();
private:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(MyTimer,logger);

HANDLE_RESULT MyTimer::on_timer_timeout()
{
	static int count = 0;
	LOG4CPLUS_DEBUG(logger, "timer timeout...count="<<count);
	++count;
	if(count > 10)
		return HANDLE_FINISH;
	else
		return HANDLE_CONTINUE;
}

int main()
{
	PropertyConfigurator::doConfigure("./conf/log4cplus.conf");

	IODemuxer *io_demuxer = new IODemuxerEpoll;
	MyTimer my_timer;
	io_demuxer->add_timer(&my_timer, 1000);
	io_demuxer->run_loop();

	return 0;
}
