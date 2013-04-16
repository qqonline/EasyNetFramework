/*
 * CondQueue.h
 *
 *  Created on: Mar 15, 2013
 *      Author: LiuYongJin
 */

#ifndef _COMMON_COND_QUEUE_H_
#define _COMMON_COND_QUEUE_H_

#include <stdint.h>
#include <pthread.h>

namespace easynet
{

//条件队列
class CondQueue
{
public:
	/**
	 * @param capacity   : 队列最大任务数
	 * @param wait_ms    : 从队列中取任务时等待到最大时间(ms).小于0:一直等待;0:不等待;大于0:等待的时间
	 */
	CondQueue(uint32_t capacity, int32_t wait_ms);
	virtual ~CondQueue();

	uint32_t GetSize();
	uint32_t GetCapacity();
	void SetWaitTime(int32_t wait_ms);


	//添加data到队列,成功返回true;失败返回false(队列满);
	bool Push(void *data);
	//从队列获取一个data,(等待wait_ms)队列空返回NULL;
	void* Pop();
private:
	uint32_t m_capacity;
	int32_t m_wait_time;
	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;

	void *m_array;
	uint32_t m_in;
	uint32_t m_out;
};

}//namespace

#endif //_COMMON_COND_QUEUE_H_


