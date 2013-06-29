/*
 * IMemory.h
 *
 *  Created on: 2013-5-18
 *      Author: LiuYongJin
 */

#ifndef _COMMON_IMEMORY_H_
#define _COMMON_IMEMORY_H_
#include <stdint.h>
#include <stdlib.h>

namespace easynet
{

class IMemory
{
public:
	virtual ~IMemory(){}

	//分配大小为size字节的内存.没有内存时返回NULL
	virtual void* Alloc(uint32_t size)=0;

	//释放大小为size的内存mem
	virtual void Free(void *mem, uint32_t size)=0;

	//将大小为size的内存mem重新分配为大小new_size.成功返回重新分配的内存,同时将mem的内容(size字节)复制到新内存中.
	virtual void* ReAlloc(void *mem, uint32_t size, uint32_t new_size)=0;
};

class SystemMemory:public IMemory
{
public:
	void* Alloc(uint32_t size)
	{
		return malloc(size);
	}

	void Free(void *mem, uint32_t size)
	{
		free(mem);
	}

	void* ReAlloc(void *mem, uint32_t size, uint32_t new_size)
	{
		return realloc(mem, new_size);
	}
};

}//namespace
#endif //_COMMON_IMEMORY_H_


