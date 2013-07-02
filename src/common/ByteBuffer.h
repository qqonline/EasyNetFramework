/*
 * ByteBuffer.h
 *
 *  Created on: May 6, 2013
 *      Author: LiuYongJin
 */
#ifndef _COMMON_BYTEBUFFER_H_TMP
#define _COMMON_BYTEBUFFER_H_TMP

#include "IMemory.h"

namespace easynet
{

class ByteBuffer
{
public:
	//构造函数
	// 默认初始化大小1k,系统内存分配器(多线程安全)
	explicit ByteBuffer();

	//构造函数
	// @param capacity : 初始化内存的大小
	// 默认使用系统内存分配器(多线程安全)
	explicit ByteBuffer(uint32_t capacity);

	//构造函数
	// @param memory   : 分配内存时使用的分配器
	// 默认初始化内存大小1k
	explicit ByteBuffer(IMemory *memory);

	//构造函数
	// @param capacity : 初始化内存的大小
	// @param memory   : 分配内存是使用的分配器
	ByteBuffer(uint32_t capacity, IMemory *memory);

	virtual ~ByteBuffer();

	//保证buffer当前的剩余空间不小于size字节(不够的话调用Enlarge扩展)
	void CheckSize(uint32_t size);

	//容量扩大size个字节,成功返回true,失败返回false(没有内存)
	bool Enlarge(uint32_t size=1024);

	char *Buffer;      //数据缓冲区
	uint32_t Capacity; //缓冲区的容量
	uint32_t Size;     //缓冲区数据大小
private:
	ByteBuffer(const ByteBuffer &buffer){}
	ByteBuffer& operator=(ByteBuffer &buffer){return *this;}

	void Init(uint32_t capacity, IMemory *memory);
private:
	static SystemMemory m_SysMemory;
	IMemory *m_Memory;   //对象的内存分配器
};

}//namespace
#endif //_COMMON_BYTEBUFFER_H_TMP
