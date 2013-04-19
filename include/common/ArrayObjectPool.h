/*
 * ArrayObjectPool.h
 *
 *  Created on: Apr 18, 2013
 *      Author: LiuYongJin
 */

#ifndef _COMMON_ARRAY_OBJECT_POOL_H_
#define _COMMON_ARRAY_OBJECT_POOL_H_

#include <stdint.h>

namespace easynet
{

class ArrayObjectPool
{
public:
	/** 构造函数
	 * @param elem_size : 元素大小(字节).小于4字节将被调整为4;
	 * @param elem_num  : 元素个数
	 */
	ArrayObjectPool(uint32_t elem_size, uint32_t elem_num);
	~ArrayObjectPool();

	uint32_t ElemSize(){return m_ElemSize;}
	uint32_t Capacity(){return m_ElemNum;}
	bool IsEmpty(){return m_FreeHead==NULL;}

	/** 获取元素
	 * @return : 返回元素指针,无空闲元素可用返回NULL;
	 */
	void* Get();

	/** 回收元素
	 * @param elem : 待回收的元素
	 * @return     : true成功;false失败,elem非本对象元素;
	 */
	bool Recycle(void *elem);
private:
	void *m_Elements;
	void *m_End;
	void *m_FreeHead;
	uint32_t m_ElemSize;
	uint32_t m_ElemNum;
};

}
#endif //_COMMON_ARRAY_OBJECT_POOL_H_


