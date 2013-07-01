/*
 * HeapSort.cpp
 *
 *  Created on: 2012-12-23
 *      Author: LiuYongJin
 */
#include <string.h>
#include <assert.h>

#include "Heap.h"

namespace easynet
{

#define INIT_CAPACITY 128

Heap::Heap(ItemCompare cmp_func)
{
	assert(cmp_func != NULL);

	m_items = (HeapItem**)malloc(sizeof(HeapItem*)*INIT_CAPACITY);
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
bool Heap::Insert(HeapItem *item)
{
	assert(item != NULL);
	if(m_size == m_capacity)
		if(_ExpandCapacity() != 0)
			return false;

	item->index = m_size;
	m_items[m_size++] = item;
	_ShiftUp(item->index);
	return true;
}

//删除堆元素.成功返回ture,失败返回false
bool Heap::Remove(HeapItem *item)
{
	assert(item != NULL);
	if(item->index<0 || item->index>=m_size)
		return false;
	HeapItem *temp = m_items[item->index] = m_items[--m_size];
	temp->index = item->index;
	int32_t result = m_cmp_func(temp, item);
	if(result == -1)
		_ShiftUp(temp->index);
	else if(result == 1)
		_ShiftDown(temp->index);
	return true;
}

//删除堆顶元素
void Heap::Pop()
{
	if(m_size <= 0)
		return;
	if(--m_size <= 0)
		return;

	m_items[0] = m_items[m_size];
	m_items[0]->index = 0;
	_ShiftDown(0);
	return ;
}

//清除堆
void Heap::Clear(ItemDestroy des_func)
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

bool Heap::_ExpandCapacity()
{
	int new_capacity = m_capacity*2;
	HeapItem **new_ptr =(HeapItem **)realloc((void*)m_items, sizeof(HeapItem*)*new_capacity);
	if(new_ptr == NULL)
		return false;
	m_items = new_ptr;
	m_capacity = new_capacity;
	return true;
}

void Heap::_ShiftUp(int32_t index)
{
	HeapItem *item = m_items[index];
	assert(index == item->index);
	int32_t parent = (index-1)/2;
	while(index>0 && parent>=0)
	{
		if(m_cmp_func(m_items[parent], item) <= 0)    //parent不大于child
			break;
		m_items[index] = m_items[parent];
		m_items[index]->index = index;
		index = parent;
		parent = (index-1)/2;
	}
	m_items[index] = item;
	item->index = index;
}

void Heap::_ShiftDown(int32_t index)
{
	HeapItem *item = m_items[index];
	assert(index == item->index);
	int32_t child = index*2+1;
	while(child < m_size)
	{
		if(child+1<m_size && m_cmp_func(m_items[child+1], m_items[child]) < 0)  //右孩子更小
			++child;
		if(m_cmp_func(item, m_items[child]) <= 0)    //比最小的孩子还小
			break;
		m_items[index] = m_items[child];
		m_items[index]->index = index;
		index = child;
		child = index*2+1;
	}
	m_items[index] = item;
	item->index = index;
}


}//namespace
