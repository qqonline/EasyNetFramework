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
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#include <bits/stl_iterator_base_types.h>
#include <bits/stl_construct.h>
using namespace std;

namespace easynet
{

//条件队列
template<typename T>
class TCondQueue
{
public:
	// @param capacity   : 队列保存的元素个数
	TCondQueue(uint32_t capacity);
	virtual ~TCondQueue();

	uint32_t GetSize();
	uint32_t GetCapacity();

	/** 添加数据到队列
	 * @param elem    : 需要保存的数据
	 * @param wait_ms : push到队列等待的时间,单位毫秒.小于0一直等到push到队列中;0无法push时立即返回;大于0等待的时间;
	 * @return        : true数据成功push到队列;false数据push失败(队列满);
	 */
	bool Push(const T &elem, int32_t wait_ms=0);

	/** 从队列获取数据
	 * @param elem    : 待返回的数据
	 * @param wait_ms : 从队列pop数据等待的时间,单位毫秒.小于0一直等到队列中有数据;0无数据时立即返回;大于0等待的时间;
	 * @return        : true成功获取到数据;false队列空,获取数据失败;
	 */
	bool Pop(T &elem, int32_t wait_ms=-1);
private:
	uint32_t         m_capacity;
	pthread_mutex_t  m_lock;
	pthread_cond_t   m_noempty_cond;
	pthread_cond_t   m_nofull_cond;

	T                *m_array;
	uint32_t         m_in;
	uint32_t         m_out;
};

template<typename T>
inline
TCondQueue<T>::TCondQueue(uint32_t capacity)
	:m_array(NULL)
	,m_in(0)
	,m_out(0)
{
	pthread_mutex_init(&m_lock, NULL);
	pthread_cond_init(&m_noempty_cond, NULL);
	pthread_cond_init(&m_nofull_cond, NULL);

	assert(capacity > 0);
	m_capacity = capacity+1;    //多出一个格子用来判断循环数组是否已满
	m_array = (T*)malloc(sizeof(T)*m_capacity);
}

template<typename T>
inline
TCondQueue<T>::~TCondQueue()
{
	pthread_mutex_destroy(&m_lock);
	pthread_cond_destroy(&m_noempty_cond);
	pthread_cond_destroy(&m_nofull_cond);

	if(m_out < m_in)
	{
		std::_Destroy(&m_array[m_out], &m_array[m_in]);
	}
	else if(m_out > m_in)
	{
		if(m_in > 0)
			std::_Destroy(&m_array[0], &m_array[m_in]);
		std::_Destroy(&m_array[0], &m_array[m_capacity]);
	}

	free((void*)m_array);
	m_array = NULL;
}

template<typename T>
inline
bool TCondQueue<T>::Push(const T &elem, int32_t wait_ms/*=0*/)
{
	pthread_mutex_lock(&m_lock);
	if((m_in+1)%m_capacity == m_out)  //已满
	{
		if(wait_ms == 0)
		{
			pthread_mutex_unlock(&m_lock);
			return false;
		}
		else if(wait_ms > 0)
		{
			struct timespec ts;
			struct timeval tv;
			uint32_t temp_us;

			gettimeofday(&tv, NULL);
			temp_us = wait_ms*1000+tv.tv_usec;             //转成微秒
			ts.tv_sec = tv.tv_sec + temp_us/1000000;       //转成秒
			ts.tv_nsec = (temp_us%1000000)*1000;           //剩下的微秒转成纳秒
			pthread_cond_timedwait(&m_nofull_cond, &m_lock, &ts);

			if((m_in+1)%m_capacity == m_out)  //超时(还是满的)再判断一次
			{
				pthread_mutex_unlock(&m_lock);
				return false;
			}
		}
		else
		{
			while((m_in+1)%m_capacity == m_out)
				pthread_cond_wait(&m_nofull_cond, &m_lock);
		}
	}

	m_array[m_in] = elem;
	m_in = (m_in+1)%m_capacity;

	pthread_mutex_unlock(&m_lock);
	pthread_cond_signal(&m_noempty_cond);
	return true;
}

template<typename T>
inline
bool TCondQueue<T>::Pop(T &elem, int32_t wait_ms/*=-1*/)
{
	pthread_mutex_lock(&m_lock);
	if(m_out == m_in)    //空
	{
		if(wait_ms == 0)    //立刻返回
		{
			pthread_mutex_unlock(&m_lock);
			return false;
		}
		if(wait_ms > 0)
		{
			struct timespec ts;
			struct timeval tv;
			uint32_t temp_us;

			gettimeofday(&tv, NULL);
			temp_us = wait_ms*1000+tv.tv_usec;             //转成微秒
			ts.tv_sec = tv.tv_sec + temp_us/1000000;       //转成秒
			ts.tv_nsec = (temp_us%1000000)*1000;           //剩下的微秒转成纳秒
			pthread_cond_timedwait(&m_noempty_cond, &m_lock, &ts);

			if(m_out == m_in)  //超时(还是空的)再判断一次
			{
				pthread_mutex_unlock(&m_lock);
				return false;
			}
		}
		else
		{
			while(m_out == m_in)
				pthread_cond_wait(&m_noempty_cond, &m_lock);
		}
	}

	elem = m_array[m_out];
	std::_Destroy(&m_array[m_out]);
	m_out = (m_out+1)%m_capacity;

	pthread_mutex_unlock(&m_lock);
	pthread_cond_signal(&m_nofull_cond);

	return true;
}

template<typename T>
inline
uint32_t TCondQueue<T>::GetSize()
{
	uint32_t size = 0;
	pthread_mutex_lock(&m_lock);
	size = (m_in>=m_out?m_in-m_out:m_in+m_capacity-m_out);
	pthread_mutex_unlock(&m_lock);
	return size;
}

template<typename T>
inline
uint32_t TCondQueue<T>::GetCapacity()
{
	uint32_t capacity = 0;
	pthread_mutex_lock(&m_lock);
	capacity = m_capacity-1;
	pthread_mutex_unlock(&m_lock);
	return capacity;
}

}//namespace

#endif //_COMMON_COND_QUEUE_H_


