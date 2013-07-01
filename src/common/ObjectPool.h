/*
 * ObjectPool.h
 *
 *  Created on: 2013-3-8
 *      Author: LiuYongJin
 */

#ifndef _COMMON_OBJECT_POOL_H_
#define _COMMON_OBJECT_POOL_H_

#include <stdint.h>

namespace easynet
{

#define OBPOOL_MAX_FREE 128

class ObjectPool
{
public:
	/** 构造函数
	 * @param object_size : 返回的内存对象大小;
	 * @param max_free    : pool中维护的最大空闲对象数;
	 */
	ObjectPool(uint32_t object_size, uint32_t max_free=OBPOOL_MAX_FREE);
	~ObjectPool();

	//从pool中获取一个对象
	void* Get();

	//回收对象到pool
	void Recycle (void *object);

private:
	const uint32_t m_object_size;
	const uint32_t m_max_free;
	uint32_t m_free_size;
	void** m_free_objects;
};

}//namespace

#endif //_COMMON_OBJECT_POOL_H_

