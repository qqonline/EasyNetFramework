/*
 * Thread.cpp
 *
 *  Created on: Mar 14, 2013
 *      Author: LiuYongJin
 */
#include <errno.h>
#include <string.h>

#include "Thread.h"

namespace easynet
{

IMPL_LOGGER(Thread, logger);

void* Thread::thread_proc(void* thread)
{
	assert(thread != NULL);
	((Thread*)thread)->do_run();
	return NULL;
}

bool Thread::start()
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if(m_stack_size > 0)  // set stack size
	{
		if(m_stack_size < MIN_STACK_SIZE)
		{
			m_stack_size = MIN_STACK_SIZE;
			LOG_DEBUG(logger, "stack size of thread too small. set to min stack size="<<MIN_STACK_SIZE);
		}
		pthread_attr_setstacksize(&attr, m_stack_size);
		LOG_DEBUG(logger, "set stack size of thread, size="<<m_stack_size);
	}
	if(m_detachable)
	{
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		LOG_DEBUG(logger, "detach thread...");
	}
	int result = pthread_create(&m_thread_id, &attr, thread_proc, (void*)this);
	pthread_attr_destroy(&attr);
	if(result != 0)
	{
		LOG_ERROR(logger, "create thread error. errno="<<errno<<"["<<strerror(errno)<<"]");
	}
	else
	{
		LOG_DEBUG(logger, "create thread success. pid="<<m_thread_id);
	}
	return result==0?true:false;
}

void Thread::wait_terminate()
{
	if(m_detachable)
		return;
	pthread_join(m_thread_id, NULL);
}

}//namespace

