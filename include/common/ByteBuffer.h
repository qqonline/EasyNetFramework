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
	//默认初始化大小1k,系统内存分配器
	explicit ByteBuffer();
	//默认使用系统内存分配器
	explicit ByteBuffer(uint32_t capacity);
	//默认初始化大小1k
	explicit ByteBuffer(IMemory *memory);
	//设置capacity和memory
	ByteBuffer(uint32_t capacity, IMemory *memory);
	virtual ~ByteBuffer();

	//容量扩大size个字节,成功返回true,失败返回false(没有内存)
	bool Enlarge(uint32_t size=1024);

	char *m_Buffer;
	uint32_t m_Capacity; //buffer的容量
	uint32_t m_Size;     //buffer中数据大小
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
