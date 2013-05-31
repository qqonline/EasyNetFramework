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
	ByteBuffer(uint32_t capacity=1024, IMemory *memory=NULL);
	~ByteBuffer();

	//容量扩大size个字节,成功返回true,失败返回false(没有内存)
	bool Enlarge(uint32_t size=1024);

	char *m_Buffer;
	uint32_t m_Capacity; //buffer的容量
	uint32_t m_Size;     //buffer中数据大小

private:
	static SystemMemory m_SysMemory;
	IMemory *m_Memory;   //对象的内存分配器
};

}//namespace
#endif //_COMMON_BYTEBUFFER_H_TMP
