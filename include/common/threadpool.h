/*
 * ThreadPool.h
 *
 *  Created on: 2012-9-9
 *      Author: LiuYongjin
 */
#ifndef _COMMON_THREAD_POOL_H_
#define _COMMON_THREAD_POOL_H_

#include <stdint.h>
#include <common/thread.h>
#include <common/logger.h>

class ThreadPool
{
public:
	ThreadPool(uint32_t thread_num);
	virtual ~ThreadPool();

	bool start();
protected:
	//创建一个线程
	virtual Thread* create_thread()=0;
private:
	Thread **m_threads;
	uint32_t m_size;
	uint32_t m_thread_num;
private:
	DECL_LOGGER(logger);
};

#endif //_COMMON_THREAD_POOL_H_



