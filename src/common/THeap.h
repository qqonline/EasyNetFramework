/*
 * THeap.h
 *
 *  Created on: Jul 5, 2013
 *      Author: tim
 */

#ifndef _COMMON_HEAP_H_
#define _COMMON_HEAP_H_

#include <stddef.h>
#include <stdint.h>
template <typename T>
class HeapLess
{
public:
	bool operator()(const T &a, const T &b) const
	{
		return a<=b;
	}
};

template <typename T, typename Cmp=HeapLess<T> >
class Heap
{
public:
	//构造函数
	// @param capacity : 堆的容量.小于0时表示不限制
	// @cmp            : 元素比较函数
	Heap(int32_t capacity);
private:
	int32_t m_Capacity;
	int32_t m_CurCapacity;
	int32_t m_Size;

	Cmp m_Compare;
};


#endif //_COMMON_HEAP_H_


