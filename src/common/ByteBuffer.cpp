/*
 * ByteBuffer.cpp
 *
 *  Created on: 2013-5-24
 *      Author: LiuYongJin
 */

#include "ByteBuffer.h"
#include <assert.h>
#include <new>

namespace easynet
{

SystemMemory ByteBuffer::m_SysMemory;

ByteBuffer::ByteBuffer(uint32_t capacity/*=1024*/, IMemory *memory/*=NULL*/)
	:m_Capacity(capacity)
	,m_Size(0)
{
	if(memory == NULL)
		memory = &m_SysMemory;
	m_Memory = memory;
	m_Buffer = (char*)m_Memory->Alloc(m_Capacity);
	assert(m_Buffer != NULL);
}

ByteBuffer::~ByteBuffer()
{
	m_Memory->Free(m_Buffer, m_Capacity);
}

bool ByteBuffer::Enlarge(uint32_t size/*=1024*/)
{
	char *temp = (char*)m_Memory->ReAlloc(m_Buffer, m_Capacity, m_Capacity+size);
	if(temp == NULL)
		return false;

	m_Buffer = temp;
	m_Capacity += size;

	return true;
}

}
