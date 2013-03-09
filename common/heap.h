/*
 * heap.h
 *
 *  Created on: 2013-03-07
 *      Author: LiuYongJin
 * Description: 堆排序模块
 */

#ifndef _COMMON_HEAP_H_
#define _COMMON_HEAP_H_

#include <stdio.h>
#include <stdint.h>

//比较函数指针.返回值:-1(a小于b); 0(a等于b); 1(a大于b)
//如果a不大于b,则a排列在b的前面
typedef int (*ElemCompare)(void *element_a, void *element_b);
//元素销毁函数指针
typedef void (*ElemDestroy)(void *element);

class Heap
{
public:
	Heap(ElemCompare cmp_func, ElemDestroy des_func);
	~Heap();

	int size();                          //堆元素个数
	bool insert(void *element);          //插入元素,成功返回0,失败返回-1
	void* top();                         //获取堆顶元素
	void pop();                          //删除堆顶元素
	void clear();                        //清除堆
private:
	int m_size;        //堆元素个数
	int m_capacity;    //堆当前容量
	void **m_elements; //堆元素数组
	ElemCompare m_cmp_func;
	ElemDestroy m_des_func;
private:
	bool _expand_capacity();
};

inline
int Heap::size()
{
	return m_size;
}

inline
void* Heap::top()
{
	return m_size>0?m_elements[0]:NULL;
}

#endif //_HEAP_SORT_H_

