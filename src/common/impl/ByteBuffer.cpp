/*
 * ByteBuffer.cpp
 *
 *  Created on: 2013-5-24
 *      Author: LiuYongJin
 */

#include "ByteBuffer.h"
#include <assert.h>

namespace easynet
{

#define INIT_CAPACITY 1024

SystemMemory ByteBuffer::m_SysMemory;

void ByteBuffer::Init(uint32_t capacity, IMemory *memory)
{
	Size = 0;
	Capacity = capacity;
	m_Memory = memory;
	Buffer = (char*)m_Memory->Alloc(Capacity);
	assert(Buffer != NULL);
}

ByteBuffer::ByteBuffer()
{
	Init(INIT_CAPACITY, &m_SysMemory);
}

ByteBuffer::ByteBuffer(uint32_t capacity)
{
	assert(capacity > 0);
	Init(capacity, &m_SysMemory);
}

ByteBuffer::ByteBuffer(IMemory *memory)
{
	assert(memory != NULL);
	Init(INIT_CAPACITY, memory);
}

ByteBuffer::ByteBuffer(uint32_t capacity, IMemory *memory)
{
	assert(capacity>0 && memory!=NULL);
	Init(capacity, memory);
}

ByteBuffer::~ByteBuffer()
{
	m_Memory->Free(Buffer, Capacity);
}

bool ByteBuffer::Enlarge(uint32_t size/*=1024*/)
{
	char *temp = (char*)m_Memory->ReAlloc(Buffer, Capacity, Capacity+size);
	if(temp == NULL)
		return false;

	Buffer = temp;
	Capacity += size;

	return true;
}

void ByteBuffer::CheckSize(uint32_t size)
{
	if(Capacity-Size < size)
		if(!Enlarge(size))
			assert(0);
}

}
