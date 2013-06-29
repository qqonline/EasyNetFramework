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
	// @param capacity   : 队列保存的元素个数
	CondQueue(uint32_t capacity);
	virtual ~CondQueue();

	uint32_t GetSize();
	uint32_t GetCapacity();

	/** 添加元素到队列,成功返回true;失败返回false(队列满);
	 * @param elem    : 需要保存的数据
	 * @param wait_ms : push到队列等待的时间,单位毫秒.小于0一直等到push到队列中;0无法push时立即返回;大于0等待的时间;
	 * @return        : true数据成功push到队列;false数据push失败;
	 */
	bool Push(void *elem, int32_t wait_ms=0);

	/** 从队列获取一个元素,(等待wait_ms)队列空返回NULL;
	 * @param wait_ms : 从队列pop数据等待的时间,单位毫秒.小于0一直等到队列中有数据;0无数据时立即返回;大于0等待的时间;
	 * @return        : 返回pop的数据,无数据时返回NULL;
	 */
	void* Pop(int32_t wait_ms=-1);
private:
	uint32_t         m_capacity;
	pthread_mutex_t  m_lock;
	pthread_cond_t   m_noempty_cond;
	pthread_cond_t   m_nofull_cond;

	void             *m_array;
	uint32_t         m_in;
	uint32_t         m_out;
};

}//namespace

#endif //_COMMON_COND_QUEUE_H_


