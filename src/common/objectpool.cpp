/*
 * objectpool.cpp
 *
 *  Created on: 2013-3-8
 *      Author: Administrator
 */

#include <common/objectpool.h>
#include <common/logger.h>
#include <assert.h>
#include <stdlib.h>

namespace easynet
{

ObjectPool::ObjectPool(uint32_t object_size, uint32_t max_free/*=OBPOOL_MAX_FREE*/)
	:m_object_size(object_size)
	,m_max_free(max_free)
	,m_free_size(0)
{
	m_free_objects = (void**)malloc(max_free*sizeof(void*));
	assert(m_free_objects != NULL);
}

ObjectPool::~ObjectPool()
{
	uint32_t i;
	for(i=0; i<m_free_size; ++i)
		free(m_free_objects[i]);
	free(m_free_objects);
}

void* ObjectPool::get()
{
	if(m_free_size > 0)
		return m_free_objects[--m_free_size];
	else
		return malloc(m_object_size);
}

void ObjectPool::recycle (void *object)
{
	if(m_free_size < m_max_free)
		m_free_objects[m_free_size++] = object;
	else
		free(object);
}


}//namespace
