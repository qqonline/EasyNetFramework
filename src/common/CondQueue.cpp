/*
 * CondQueue.cpp
 *
 *  Created on: Mar 15, 2013
 *      Author: LiuYongJin
 */

#include <common/CondQueue.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

namespace easynet
{


typedef struct _cq_item
{
	void *data;
	struct _cq_item *next;
}CQItem;

CondQueue::CondQueue(uint32_t capacity, uint32_t wait_ms, uint32_t cache_size/*=0*/)
	:m_capacity(capacity)
	,m_wait_time(wait_ms)
	,m_size(0)
	,m_queue_head(NULL)
	,m_data_pool(NULL)
{
	pthread_mutex_init(&m_lock, NULL);
	pthread_cond_init(&m_cond, NULL);
	if(cache_size > 0)
		m_data_pool = new ObjectPool(sizeof(CQItem), cache_size);
}

CondQueue::~CondQueue()
{
	pthread_mutex_destroy(&m_lock);
	pthread_cond_destroy(&m_cond);
	if(m_data_pool != NULL)
		delete m_data_pool;
	m_data_pool = NULL;
}

bool CondQueue::Push(void *data)
{
	assert(data != NULL);
	pthread_mutex_lock(&m_lock);
	if(m_size >= m_capacity)
	{
		pthread_mutex_unlock(&m_lock);
		return false;
	}
	CQItem *item = NULL;
	if(m_data_pool != NULL)
		item = (CQItem*)m_data_pool->Get();
	else
		item = new CQItem;
	if(item == NULL)
	{
		pthread_mutex_unlock(&m_lock);
		return false;
	}
	item->data = data;
	item->next = (CQItem*)m_queue_head;
	m_queue_head = (void*)item;
	++m_size;
	pthread_mutex_unlock(&m_lock);
	pthread_cond_signal(&m_cond);
	return true;
}

void* CondQueue::Pop()
{
	void *data = NULL;
	pthread_mutex_lock(&m_lock);
	if(m_size <= 0)
	{
		if(m_wait_time == 0)    //立刻返回
		{
			pthread_mutex_unlock(&m_lock);
			return NULL;
		}
		if(m_wait_time > 0)
		{
			struct timespec ts;
			struct timeval tv;
			gettimeofday(&tv, NULL);
			uint32_t temp = m_wait_time+tv.tv_usec/1000;
			ts.tv_sec = tv.tv_sec + temp/1000;
			ts.tv_nsec = (temp%1000)*1000000;
			pthread_cond_timedwait(&m_cond, &m_lock, &ts);
		}
		else
		{
			pthread_cond_wait(&m_cond, &m_lock);
		}

		if(m_size > 0)
		{
			assert(m_queue_head != NULL);
			data = ((CQItem*)m_queue_head)->data;
			CQItem *next = ((CQItem*)m_queue_head)->next;
			if(m_data_pool != NULL)
				m_data_pool->Recycle(m_queue_head);
			else
				delete (CQItem*)m_queue_head;
			m_queue_head = (void*)next;
			--m_size;
		}
	}
	pthread_mutex_unlock(&m_lock);
	return data;
}

uint32_t CondQueue::GetSize()
{
	uint32_t size = 0;
	pthread_mutex_lock(&m_lock);
	size = m_size;
	pthread_mutex_unlock(&m_lock);
	return size;
}

uint32_t CondQueue::GetCapacity()
{
	uint32_t capacity = 0;
	pthread_mutex_lock(&m_lock);
	capacity = m_capacity;
	pthread_mutex_unlock(&m_lock);
	return capacity;
}

void CondQueue::SetCapacity(uint32_t capacity)
{
	pthread_mutex_lock(&m_lock);
	m_capacity = capacity;
	pthread_mutex_unlock(&m_lock);
}

void CondQueue::SetWaitTime(uint32_t wait_ms)
{
	pthread_mutex_lock(&m_lock);
	m_wait_time = wait_ms;
	pthread_mutex_unlock(&m_lock);
}


}//namespace
