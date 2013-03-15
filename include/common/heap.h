/*
 * heap.h
 *
 *  Created on: 2013-03-07
 *      Author: LiuYongJin
 * Description: 堆排序模块
 */

#ifndef _COMMON_HEAP_H_
#define _COMMON_HEAP_H_

#include <stdlib.h>
#include <stdint.h>

namespace easynet
{

typedef struct _heap_item_
{
	uint32_t index;
}HeapItem;

//比较函数指针.返回值:小于0(a小于b); 0(a等于b); 大于0(a大于b)
//如果a不大于b,则a排列在b的前面
typedef int (*ItemCompare)(HeapItem *item0, HeapItem *item1);
//元素销毁函数指针
typedef void (*ItemDestroy)(HeapItem *item);

class Heap
{
public:
	Heap(ItemCompare cmp_func);
	~Heap();

	int size();                          //堆元素个数
	bool insert(HeapItem *item);         //插入元素.成功返回ture,失败返回false
	bool remove(HeapItem *item);         //删除元素.成功返回ture,失败返回false
	HeapItem* top();                     //获取堆顶元素
	void pop();                          //删除堆顶元素
	void clear(ItemDestroy des_func);    //清除堆(如果指定destroy函数,则用该函数处理每个heap item)
	HeapItem* get_item(uint32_t index);  //获取指定的heap item
private:
	int m_size;              //堆元素个数
	int m_capacity;          //堆当前容量
	HeapItem **m_items;      //堆元素数组
	ItemCompare m_cmp_func;  //堆元素比较函数
private:
	void _shift_up(uint32_t index);      //自底向上调整
	void _shift_down(uint32_t index);    //自顶向下调整
	bool _expand_capacity();             //扩展堆的大小
};

inline
int Heap::size()
{
	return m_size;
}

inline
HeapItem* Heap::top()
{
	return m_size>0?m_items[0]:NULL;
}

inline
HeapItem* Heap::get_item(uint32_t index)
{
	return index>=m_size?NULL:m_items[index];
}

}//namespace

#endif //_HEAP_SORT_H_

