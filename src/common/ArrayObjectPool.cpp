/*
 * ArrayObjectPool.cpp
 *
 *  Created on: Apr 18, 2013
 *      Author: LiuYongJin
 */
#include <assert.h>
#include <stddef.h>

#include "common/ArrayObjectPool.h"

namespace easynet
{

ArrayObjectPool::ArrayObjectPool(uint32_t elem_size, uint32_t elem_num)
{
	m_ElemNum = elem_num;
	if(elem_size < sizeof(void*))
		m_ElemSize = sizeof(void*);
	else
		m_ElemSize = elem_size;

	m_Elements = malloc(m_ElemSize*m_ElemNum);
	m_End = (void*)((char*)m_Elements+m_ElemSize*m_ElemNum);
	assert(m_Elements != NULL);

	//构建链表
	int i;
	void *node = m_Elements;
	for(i=0; i<m_ElemNum-1; ++i)
	{
		*(void**)node = (void*)((char*)node+m_ElemSize);
		node = *(void**)node;
	}
	*(void**)node = NULL;
	m_FreeHead = m_Elements;    //链表头
}

ArrayObjectPool::~ArrayObjectPool()
{
	free(m_Elements);
}

void* ArrayObjectPool::Get()
{
	if(m_FreeHead == NULL)
		return NULL;
	void *temp = m_FreeHead;
	m_FreeHead = *(void**)m_FreeHead;
	return temp;
}

bool ArrayObjectPool::Recycle(void *elem)
{
	if(elem<m_Elements || elem>=m_End)
		return false;
	*(void**)elem = m_FreeHead;
	m_FreeHead = elem;
	return true;
}

}

