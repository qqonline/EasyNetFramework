/*
 * KVData.cpp
 *
 *  Created on: Apr 26, 2013
 *      Author: LiuYongJin
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "KVData.h"

namespace easynet
{

#define KV_CAPACITY    512    //初始化大小为512Bytes

#define TYPE_I8        0
#define TYPE_I16       1
#define TYPE_I32       2
#define TYPE_I64       3
#define TYPE_BYTES     4
#define TYPE_BY0       4    //byte,多出0字节
#define TYPE_BY1       5    //byte,多出1字节,需要填充3字节
#define TYPE_BY2       6    //byte,多出2字节,需要填充2字节
#define TYPE_BY3       7    //byte,多出3字节,需要填充1字节

#define KeyType(k, t)  (k<<3|t)
#define ToKey(ky)      (ky>>3)
#define ToType(ky)     (ky&0x07)


#ifndef  htonll
#define  htonll(x)     (((uint64_t)(htonl((uint32_t)((x)&0xffffffff)))<<32) | htonl((uint32_t)(((x)>>32)&0xffffffff)))
#define  ntohll(x)     (((uint64_t)(ntohl((uint32_t)((x)&0xffffffff)))<<32) | ntohl((uint32_t)(((x)>>32)&0xffffffff)))
#endif//hton64

KVData::KVData():m_Capacity(KV_CAPACITY)
{
	m_Memory = (IMemory*)&m_DefaultMemory;
	m_Buffer = m_Memory->Alloc(m_Capacity);
	assert(m_Buffer != NULL);
	_InitMagicNum();
}

KVData::KVData(IMemory *memory):m_Capacity(KV_CAPACITY), m_Memory(memory)
{
	assert(m_Memory != NULL);
	m_Buffer = m_Memory->Alloc(m_Capacity);
	assert(m_Buffer != NULL);
	_InitMagicNum();
}

KVData::~KVData()
{
	m_Memory->Free(m_Buffer, m_Capacity);
}

void KVData::_InitMagicNum()
{
	char *magic = (char*)m_Buffer;
	magic[0] = 'K';
	magic[1] = 'V';
	magic[2] = 'D';
	magic[3] = 'T';
	m_Size = 4;
}

bool KVData::_Set(uint16_t key, uint16_t type, void *bytes, uint32_t size)
{
	uint32_t len = sizeof(uint32_t);   //key+type
	if(type == TYPE_I32)
		len += sizeof(int32_t);
	else if(type == TYPE_I64)
		len += sizeof(int64_t);
	else if(type == TYPE_BYTES)
	{
		len += sizeof(int32_t)+size;
		uint32_t temp = len%sizeof(int32_t);
		type += temp;
		len += (sizeof(int32_t)-temp)%sizeof(int32_t);  //对齐,填充
	}

	if(m_Size+len > m_Capacity)  //重新分配内存
	{
		uint32_t new_size = m_Size+len;
		uint32_t capacity = (new_size/KV_CAPACITY)*KV_CAPACITY;
		if(new_size%KV_CAPACITY != 0)
			capacity += KV_CAPACITY;

		void *temp = m_Memory->ReAlloc(m_Buffer, m_Capacity, capacity);
		if(temp == NULL)
			return false;
		m_Buffer = temp;
		m_Capacity = capacity;
	}

	char *ptr = (char*)m_Buffer+m_Size;
	*(uint16_t*)ptr = htons(KeyType(key, type));
	ptr += sizeof(uint16_t);
	switch(type)
	{
	case TYPE_I8:
	{
		*(int8_t*)ptr = *(int8_t*)bytes;
		break;
	}
	case TYPE_I16:
	{
		int16_t temp = *(int16_t*)bytes;
		*(int16_t*)ptr =  (int16_t)htons(temp);
		break;
	}
	case TYPE_I32:
	{
		ptr += sizeof(int16_t);
		int32_t temp = *(int32_t*)bytes;
		*(int32_t*)ptr = (int32_t)htonl(temp);
		break;
	}
	case TYPE_I64:
	{
		ptr += sizeof(int16_t);
		int64_t temp = *(int64_t*)bytes;
		*(int64_t*)ptr = (int64_t)htonll(temp);
		break;
	}
	case TYPE_BY0:
	case TYPE_BY1:
	case TYPE_BY2:
	case TYPE_BY3:
	{
		ptr += sizeof(int16_t);
		*(uint32_t*)ptr = htonl(size);
		ptr += sizeof(uint32_t);
		memcpy(ptr, bytes, size);
		break;
	}
	default:
		return false;
	}
	m_Size += len;
	return true;
}

//8
bool KVData::Set(uint16_t key, int8_t val)
{
	return _Set(key, TYPE_I8, (void*)&val, sizeof(int8_t));
}

//16
bool KVData::Set(uint16_t key, int16_t val)
{
	return _Set(key, TYPE_I16, (void*)&val, sizeof(int16_t));
}

//32
bool KVData::Set(uint16_t key, int32_t val)
{
	return _Set(key, TYPE_I32, (void*)&val, sizeof(int32_t));
}

//64
bool KVData::Set(uint16_t key, int64_t val)
{
	return _Set(key, TYPE_I64, (void*)&val, sizeof(int64_t));
}

//bytes
bool KVData::Set(uint16_t key, const void *bytes, uint32_t size)
{
	if(bytes==NULL || size==0)
		return false;
	return _Set(key, TYPE_BYTES, (void*)bytes, size);
}

bool KVData::Set(uint16_t key, const char *c_str)
{
	uint32_t len = 0;
	if(c_str == NULL)
		return false;
	if((len=strlen(c_str)) == 0)
		return false;
	return _Set(key, TYPE_BYTES, (void*)c_str, len+1);
}

bool KVData::UnPack()
{
	char *temp = (char*)m_Buffer;
	if(m_Size<4 || temp[0]!='K' || temp[1]!='V' || temp[2]!='D' || temp[3]!='T')
		return false;
	m_PosMap.clear();
	char *ptr_start = (char*)m_Buffer+4;
	char *ptr_end   = (char*)m_Buffer+m_Size;
	while(ptr_start < ptr_end)
	{
		void *pos = (void*)ptr_start;
		//key
		if(ptr_start+sizeof(uint32_t) > ptr_end)
			return false;
		uint16_t key_type = ntohs(*(uint16_t*)ptr_start);
		ptr_start += sizeof(uint32_t);     //next block
		uint16_t key = ToKey(key_type);    //key
		uint16_t type = ToType(key_type);  //type
		switch(type)
		{
		case TYPE_I8:                 //8
		case TYPE_I16:                //16
			break;
		case TYPE_I32:                //32
			ptr_start += sizeof(int32_t);
			break;
		case TYPE_I64:                //64
			ptr_start += sizeof(int64_t); break;
		case TYPE_BY0: case TYPE_BY1: case TYPE_BY2: case TYPE_BY3:  //bytes
		{
			if(ptr_start+sizeof(uint32_t) > ptr_end)
				return false;
			uint32_t size = *(uint32_t*)ptr_start;
			size = ntohl(size);
			if(size == 0)
				return false;
			ptr_start += sizeof(uint32_t);
			type -= TYPE_BYTES;    //多出的字节数
			size += (sizeof(uint32_t)-type)%sizeof(uint32_t);  //对齐
			ptr_start += size;
			break;
		}
		default:
			return false;
		}

		PosMap::iterator it = m_PosMap.find(key);
		if(it == m_PosMap.end())
			m_PosMap.insert(std::make_pair(key, pos));
		else
			it->second = pos;
	}

	return true;
}

//32
bool KVData::Get(uint16_t key, int32_t *val)
{
	PosMap::iterator it = m_PosMap.find(key);
	if(it == m_PosMap.end())
		return false;
	char *ptr = (char*)(it->second);
	uint16_t key_type = *(uint16_t*)ptr;
	key_type = ntohs(key_type);
	ptr += sizeof(uint16_t);
	uint16_t rkey = ToKey(key_type);
	uint16_t rtype = ToType(key_type);

	if(rtype == TYPE_I8)
		*val = (int32_t)*(int8_t*)ptr;
	else if(rtype == TYPE_I16)
		*val = (int32_t)ntohs(*(int16_t*)ptr);
	else if(rtype == TYPE_I32)
		*val = (int32_t)ntohs(*(int32_t*)ptr);
	else
		return false;
	return true;
}

//64
bool KVData::Get(uint16_t key, int64_t *val)
{
	PosMap::iterator it = m_PosMap.find(key);
	if(it == m_PosMap.end())
		return false;
	char *ptr = (char*)(it->second);
	uint16_t key_type = *(uint16_t*)ptr;
	key_type = ntohs(key_type);
	ptr += sizeof(uint16_t);
	uint16_t rkey = ToKey(key_type);
	uint16_t rtype = ToType(key_type);

	if(rtype == TYPE_I64)
	{
		*val = (int64_t)ntohll(*(int64_t*)ptr);
		return true;
	}
	return false;
}

//bytes
bool KVData::Get(uint16_t key, void **bytes, uint32_t *size)
{
	if(bytes == NULL || size==NULL)
		return false;

	PosMap::iterator it = m_PosMap.find(key);
	if(it == m_PosMap.end())
		return false;
	char *ptr = (char*)(it->second);
	uint16_t key_type = *(uint16_t*)ptr;
	key_type = ntohs(key_type);
	ptr += sizeof(uint16_t);
	uint16_t rkey = ToKey(key_type);
	uint16_t rtype = ToType(key_type);

	if(rtype<TYPE_BY0 || rtype>TYPE_BY3)
		return false;

	rtype = TYPE_BYTES;
	ptr += sizeof(uint16_t);

	uint32_t temp = *(uint32_t*)ptr;
	*size = ntohl(temp);
	if(*size == 0)
		return false;
	ptr += sizeof(uint32_t);
	*bytes = (void*)ptr;
	return true;
}

}//namespace
