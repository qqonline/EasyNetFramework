/*
 * Thread.h
 *
 *  Created on: 2013-03-15
 *      Author: LiuYongjin
 */

#ifndef _COMMON_THREAD_H_
#define _COMMON_THREAD_H_

#include <stdint.h>
#include <pthread.h>
#include <assert.h>

#include "Logger.h"

namespace easynet
{

//最小线程栈2M
#define MIN_STACK_SIZE 2*1024*1204

class Thread
{
static void* ThreadProc(void* thread);
public:
	Thread(bool detachable=true, uint32_t stack_size=0)
		:m_detachable(detachable)
		,m_stack_size(stack_size)
		,m_thread_id(0)
	{}
	virtual ~Thread(){}

	//启动线程
	bool Start();
	//等待线程结束
	void WaitTerminate();

protected:
	//线程实际运行接口
	virtual void DoRun()=0;
private:
	pthread_t m_thread_id;
	bool m_detachable;
	uint32_t m_stack_size;

private:
	DECL_LOGGER(logger);
};

}//namespace

#endif //_COMMON_THREAD_H_

