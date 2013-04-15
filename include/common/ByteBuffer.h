/*
 * ByteBuffer.h
 *
 *  Created on: 2013-3-14
 *      Author: LiuYongJin
 */

#ifndef _COMMON_BYTEBUFFER_H_
#define _COMMON_BYTEBUFFER_H_

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

namespace easynet
{

#define BF_INIT_CAPACITY 512

class ByteBuffer
{
public:
	ByteBuffer(uint32_t init_capacity=BF_INIT_CAPACITY);
	~ByteBuffer();

	//获取整个buffer
	char* GetBuffer(){return m_base;}
	//获取总数据大小
	uint32_t GetSize(){return m_size;}

	//从有效数据后面获取一个大小为size的buffer用于添加数据
	char* GetAppendBuffer(uint32_t size);
	//设置添加到buffer中的大小
	void SetAppendSize(uint32_t size);

	//返回可获取的数据大小
	uint32_t GetFetchSize();
	//获取有效数据的buffer用于获取数据,返回有效数据的size;没有数据返回NULL,size设置为0
	char* GetFetchBuffer(uint32_t *size=NULL);
	//设置从buffer中取出的数据大小;
	void SetFetchSize(uint32_t size);
public:
	//添加大小为size的bytes数组到buffer后面
	bool AppendBytes(char *bytes, uint32_t size);
	//添加字符串(不包含'\0')到buffer后面
	bool AppendStr(char *str);
	//添加32位整数i到buffer后面
	bool AppendInt32(int32_t i);
	//添加64位整数到buffer后面
	bool AppendInt64(int64_t i);

	//严格从buffer读出size字节到buf中.数据不足size字节返回false.
	bool FetchBytes(char *buf, uint32_t size);
	//严格从buffer读出32位整数.数据不足size字节返回false.
	bool FetchInt32(int32_t *i);
	//严格从buffer读取64位整数.数据不足size字节返回false.
	bool FetchInt64(int64_t *i);
private:
	char *m_base;
	uint32_t m_capacity;
	uint32_t m_size;
	uint32_t m_fetch_pos;
};

inline
void ByteBuffer::set_append_size(uint32_t size)
{
	assert(m_size+size <= m_capacity);
	m_size += size;
}

inline
void ByteBuffer::set_fetch_size(uint32_t size)
{
	assert(m_fetch_pos+size <= m_size);
	m_fetch_pos += size;
}

inline
bool ByteBuffer::append_str(char *str)
{
	return str==NULL?true:append_bytes(str, strlen(str));
}

inline
bool ByteBuffer::append_int32(int32_t i)
{
	return append_bytes((char*)&i, sizeof(i));
}

inline
bool ByteBuffer::append_int64(int64_t i)
{
	return append_bytes((char*)&i, sizeof(i));
}

inline
uint32_t ByteBuffer::get_fetch_size()
{
	return m_fetch_pos<m_size?m_size-m_fetch_pos:0;
}

bool ByteBuffer::fetch_int32(int32_t *i)
{
	return i==NULL?false:fetch_bytes((char*)i, sizeof(int32_t));
}

bool ByteBuffer::fetch_int64(int64_t *i)
{
	return i==NULL?false:fetch_bytes((char*)i, sizeof(int64_t));
}


}//namespace

#endif //_COMMON_BYTEBUFFER_H_


