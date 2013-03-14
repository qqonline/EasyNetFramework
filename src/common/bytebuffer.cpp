/*
 * bytebuffer.cpp
 *
 *  Created on: 2013-3-14
 *      Author: LiuYongJin
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <common/bytebuffer.h>

ByteBuffer::ByteBuffer(uint32_t init_capacity/*=BF_INIT_CAPACITY*/)
	:m_capacity(init_capacity)
	,m_size(0)
	,m_fetch_pos(0)
{
	m_base = (char*)calloc(m_capacity, 1);
	assert(m_base != NULL);
}

ByteBuffer::~ByteBuffer()
{
	free(m_base);
	m_base = NULL;
	m_capacity = 0;
	m_size = 0;
}

//从有效数据后面获取一个大小为size的buffer用于添加数据
char* ByteBuffer::get_append_buffer(uint32_t size)
{
	assert(size > 0);
	if(m_size+size > m_capacity)
	{
		uint32_t new_cap = m_capacity;
		do{
			new_cap <<= 1;
		}while(m_size+size > new_cap);

		char* new_base = (char*)realloc((void*)m_base, new_cap);
		if(new_base == NULL)
			return NULL;
		m_base = new_base;
		m_capacity = new_cap;
	}
	return m_base+m_size;
}

//获取有效数据的buffer用于获取数据,返回有效数据的size;
char* ByteBuffer::get_fetch_buffer(uint32_t *size/*=NULL*/)
{
	if(size != NULL)
		*size = 0;

	if(m_fetch_pos >= m_size)
		return NULL;
	if(size != NULL)
		*size = m_size-m_fetch_pos;
	return m_base+m_fetch_pos;
}

bool ByteBuffer::append_bytes(char *bytes, uint32_t size)
{
	if(bytes==NULL || size==0)
		return true;

	char *temp = get_append_buffer(size);
	if(temp == NULL)
		return false;
	memcpy(temp, bytes, size);
	set_append_size(size);
	return true;
}

bool ByteBuffer::fetch_bytes(char *buf, uint32_t size)
{
	assert(buf != NULL);
	uint32_t len;
	char *temp = get_fetch_buffer(&len);
	if(temp==NULL || len<size)
		return false;
	memcpy(buf, temp, size);
	set_fetch_size(size);
	return true;
}
