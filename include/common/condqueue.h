/*
 * condqueue.h
 *
 *  Created on: Mar 15, 2013
 *      Author: LiuYongJin
 */

#ifndef _COMMON_COND_QUEUE_H_
#define _COMMON_COND_QUEUE_H_

#include <stdint.h>
#include <pthread.h>
#include <common/objectpool.h>

//条件队列
class CondQueue
{
public:
	//capacity: 队列最大任务数
	//wait_ms:从队列中取任务时等待到最大时间(ms)
	CondQueue(int32_t capacity, int32_t wait_ms);
	virtual ~CondQueue();

	void set_capacity(int32_t capacity);
	void set_wait_time(int32_t wait_ms);

	//添加data到队列,成功返回true;失败返回false(队列满);
	bool push(void *data);
	//从队列获取一个data,(等待wait_ms)队列空返回NULL;
	void* pop();
private:
	int32_t m_size;
	int32_t m_capacity;
	int32_t m_wait_time;
	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;

	void *m_queue_head;
	ObjectPool m_data_pool;
};

#endif //_COMMON_COND_QUEUE_H_


