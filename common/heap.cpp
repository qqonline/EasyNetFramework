/*
 * HeapSort.cpp
 *
 *  Created on: 2012-12-23
 *      Author: LiuYongJin
 */
#include "heap.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define INIT_CAPACITY 128

Heap::Heap(ElemCompare cmp_func, ElemDestroy des_func)
{
	assert(cmp_func != NULL);

	m_elements = (void **)malloc(sizeof(void*)*INIT_CAPACITY);
	assert(m_elements != NULL);

	m_size = 0;
	m_capacity = INIT_CAPACITY;
	m_cmp_func = cmp_func;
	m_des_func = des_func;
}

Heap::~Heap()
{
	if(m_des_func != NULL)
		while(m_size > 0)
			m_des_func(m_elements[--m_size]);
	free(m_elements);
	m_elements = NULL;
}

//插入元素,成功返回0,失败返回-1
bool Heap::insert(void *element)
{
	if(m_size == m_capacity)
		if(_expand_capacity() != 0)
			return false;

	//自底向上调整
	int curpos = m_size;
	int p = (curpos-1)/2;
	while(curpos>0 && p>=0)
	{
		if(m_cmp_func(m_elements[p], element) != 1)
			break;
		m_elements[curpos] = m_elements[p];
		curpos = p;
		p = (curpos-1)/2;
	}
	m_elements[curpos] = element;
	++m_size;

	return true;
}

//删除堆顶元素
void Heap::pop()
{
	if(m_size <= 0)
		return;

	if(m_des_func != NULL)
		m_des_func(m_elements[0]);
	--m_size;
	if(m_size <= 0)
		return;

	void *element = m_elements[m_size];
	int curpos = 0;
	int child = curpos*2+1;
	while(child < m_size)
	{
		if(child+1<m_size && m_cmp_func(m_elements[child+1], m_elements[child]) == -1)
			++child;
		if(m_cmp_func(element, m_elements[child]) != 1)
			break;
		m_elements[curpos] = m_elements[child];
		curpos = child;
		child = curpos*2+1;
	}
	m_elements[curpos] = element;
}

//清除堆
void Heap::clear()
{
	if(m_des_func != NULL)
		while(m_size > 0)
			m_des_func(m_elements[--m_size]);
	m_size = 0;

	if(m_capacity > INIT_CAPACITY)
	{
		m_capacity = INIT_CAPACITY;
		m_elements = (void **)realloc((void*)m_elements, sizeof(void*)*m_capacity);
	}
}

bool Heap::_expand_capacity()
{
	int new_capacity = m_capacity * 2;
	void ** new_ptr =(void **)realloc((void*)m_elements, sizeof(void*)*new_capacity);
	if(new_ptr == NULL)
		return false;
	m_elements = new_ptr;
	m_capacity = new_capacity;
	return true;
}
