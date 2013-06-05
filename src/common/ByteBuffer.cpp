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

void ByteBuffer::Init(uint32_t capacity, IMemory *memory)
{
	m_Size = 0;
	m_Memory = memory;
	m_Buffer = (char*)m_Memory->Alloc(m_Capacity);
	assert(m_Buffer != NULL);
}

ByteBuffer::ByteBuffer()
{
	Init(1024, &m_SysMemory);
}

ByteBuffer::ByteBuffer(uint32_t capacity)
{
	assert(capacity > 0);
	Init(capacity, &m_SysMemory);
}

ByteBuffer::ByteBuffer(IMemory *memory)
{
	assert(memory != NULL);
	Init(1024, memory);
}

ByteBuffer::ByteBuffer(uint32_t capacity, IMemory *memory)
{
	assert(capacity>0 && memory!=NULL);
	Init(capacity, memory);
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
