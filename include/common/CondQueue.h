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
#include <common/ObjectPool.h>

namespace easynet
{

//条件队列
class CondQueue
{
public:
	/**
	 * @param capacity   : 队列最大任务数
	 * @param wait_ms    : 从队列中取任务时等待到最大时间(ms)
	 * @param cache_size : 使用cache时,内部节点cache个数.默认不进行cache.
	 */
	CondQueue(uint32_t capacity, uint32_t wait_ms, uint32_t cache_size=0);
	virtual ~CondQueue();

	uint32_t GetSize();
	uint32_t GetCapacity();
	void SetCapacity(uint32_t capacity);
	void SetWaitTime(uint32_t wait_ms);


	//添加data到队列,成功返回true;失败返回false(队列满);
	bool Push(void *data);
	//从队列获取一个data,(等待wait_ms)队列空返回NULL;
	void* Pop();
private:
	int32_t m_size;
	uint32_t m_capacity;
	uint32_t m_wait_time;
	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;

	void *m_queue_head;
	ObjectPool *m_data_pool;
};

}//namespace

#endif //_COMMON_COND_QUEUE_H_


