/*
 * objectpool.h
 *
 *  Created on: 2013-3-8
 *      Author: LiuYongJin
 */

#ifndef _COMMON_OBJECT_POOL_H_
#define _COMMON_OBJECT_POOL_H_


#define OBPOOL_MAX_FREE 128

class ObjectPool
{
public:
	//object_size: 返回的内存对象大小;
	//max_free:pool中维护的最大空闲对象数;
	ObjectPool(int object_size, int max_free=OBPOOL_MAX_FREE);

	~ObjectPool();

	//从pool中获取一个对象
	void* get();

	//回收对象到pool
	void recycle (void *object);

private:
	const int m_object_size;
	const int m_max_free;
	int m_free_size;
	void** m_free_objects;
};



#endif //_COMMON_OBJECT_POOL_H_

