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

Heap::Heap(ItemCompare cmp_func)
{
	assert(cmp_func != NULL);

	m_items = (void **)malloc(sizeof(HeapItem*)*INIT_CAPACITY);
	assert(m_items != NULL);

	m_size = 0;
	m_capacity = INIT_CAPACITY;
	m_cmp_func = cmp_func;
}

Heap::~Heap()
{
	free(m_items);
	m_items = NULL;
}

//插入元素.成功返回ture,失败返回false
bool Heap::insert(HeapItem *item)
{
	assert(item != NULL);
	if(m_size == m_capacity)
		if(_expand_capacity() != 0)
			return false;

	item->index = m_size;
	m_items[m_size++] = item;
	_shift_up(item->index);
	return true;
}

//删除堆元素.成功返回ture,失败返回false
bool Heap::remove(HeapItem *item)
{
	assert(item != NULL);
	if(item->index >= m_size)
		return false;
	HeapItem *temp = m_items[item->index] = m_items[--m_size];
	temp->index = item->index;
	int result = m_cmp_func(temp, item);
	if(result == -1)
		_shift_up(temp->index);
	else if(result == 1)
		_shift_down(temp->index);
	return true;
}

//删除堆顶元素
void Heap::pop()
{
	if(m_size <= 0)
		return;
	if(--m_size <= 0)
		return;

	m_items[0] = m_items[m_size];
	m_items[0]->index = 0;
	_shift_down(0);
	return ;
}

//清除堆
void Heap::clear(ItemDestroy des_func)
{
	if(des_func != NULL)
		while(m_size > 0)
			des_func(m_items[--m_size]);
	m_size = 0;
	if(m_capacity > INIT_CAPACITY)
	{
		m_capacity = INIT_CAPACITY;
		m_items = (HeapItem **)realloc((void*)m_items, sizeof(HeapItem*)*m_capacity);
	}
}

bool Heap::_expand_capacity()
{
	int new_capacity = m_capacity*2;
	HeapItem **new_ptr =(HeapItem **)realloc((void*)m_items, sizeof(HeapItem*)*new_capacity);
	if(new_ptr == NULL)
		return false;
	m_items = new_ptr;
	m_capacity = new_capacity;
	return true;
}

void Heap::_shift_up(uint32_t index)
{
	HeapItem *item = m_items[index];
	assert(index == item->index);
	int parent = (index-1)/2;
	while(index>0 && parent>=0)
	{
		if(m_cmp_func(m_items[parent], item) != 1)    //parent不大于child
			break;
		m_items[index] = m_items[parent];
		m_items[index]->index = index;
		index = parent;
		parent = (index-1)/2;
	}
	m_items[index] = item;
	item->index = index;
}

void Heap::_shift_down(uint32_t index)
{
	HeapItem *item = m_items[index];
	assert(index == item->index);
	int child = index*2+1;
	while(child < m_size)
	{
		if(child+1<m_size && m_cmp_func(m_items[child+1], m_items[child])==-1)  //右孩子更小
			++child;
		if(m_cmp_func(item, m_items[child]) != 1)    //比最小的孩子还小
			break;
		m_items[index] = m_items[child];
		m_items[index]->index = index;
		index = child;
		child = index*2+1;
	}
	m_items[index] = item;
	item->index = index;
}
