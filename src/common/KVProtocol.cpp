/*
 * KVProtocol.cpp
 *
 *  Created on: Apr 26, 2013
 *      Author: LiuYongJin
 */
#include <stdlib.h>
#include <assert.h>

#include "KVProtocol.h"

namespace easynet
{

IMPL_LOGGER(KVProtocol, logger);

#define KV_CAPACITY   1024    //初始化大小为1k

#define TYPE_I8    0
#define TYPE_I16   1
#define TYPE_I32   2
#define TYPE_I64   3
#define TYPE_BYTES 4
#define TYPE_BP0   4    //byte,填充0字节
#define TYPE_BP1   5    //byte,填充1字节
#define TYPE_BP2   6    //byte,填充2字节
#define TYPE_BP3   7    //byte,填充3字节

#define KeyType(k, t)   (k<<3|t)
#define ToKey(ky)       (ky>>3)
#define ToType(ky)      (ky&0x07)


KVProtocol::KVProtocol()
	:m_Size(0)
	,m_Capacity(KV_CAPACITY)
{
	m_Buffer = (uint8_t*)calloc(m_Capacity, 1);
	assert(m_Buffer != NULL);
}

bool KVProtocol::_Set(uint16_t key, uint16_t type, int8_t *bytes, uint32_t size)
{
	uint32_t len = sizeof(uint32_t);
	if(type > TYPE_I16)
		len += sizeof(uint32_t);

	if(type == TYPE_I64)
		len += sizeof(uint32_t);
	else if(type == TYPE_BYTES)
	{
		len += size;
		type += len%sizeof(uint32_t);
		len += sizeof(uint32_t)-len%sizeof(uint32_t);  //对齐,填充
	}

	if(m_Size+len>m_Capacity && !_ExpandCapacity(m_Size+len))
		return false;

	char *ptr = m_Buffer+m_Size;
	*(uint16_t*)ptr = KeyType(key, type);
	ptr += sizeof(uint16_t);
	switch(type)
	{
	case TYPE_I8:   *(int8_t)ptr = *(int8_t*)bytes;    break;
	case TYPE_I16:  *(int16_t)ptr = *(int16_t*)bytes;  break;
	case TYPE_I32:  ptr += sizeof(uint16_t); *(int32_t)ptr = *(int32_t*)bytes;  break;
	case TYPE_I64:  ptr += sizeof(uint16_t); *(int64_t)ptr = *(int64_t*)bytes;  break;
	case TYPE_BP0:
	case TYPE_BP1:
	case TYPE_BP2:
	case TYPE_BP3:
		ptr += sizeof(uint16_t);
		*(uint32_t)ptr = size;
		ptr += sizeof(uint32_t);
		memcpy((void*)ptr, bytes, size);
		break;
	}
	return true;
}

bool KVProtocol::_ExpandCapacity(uint32_t need_size)
{
	uint32_t capacity = (1+need_size/KV_CAPACITY)*KV_CAPACITY;
	if(capacity-need_size < KV_CAPACITY/2)
		capacity += KV_CAPACITY;

	uint8_t *temp = (uint8_t*)realloc(m_Buffer, capacity);
	if(temp == NULL)
	{
		LOG_ERROR(logger, "kvprotocol out of memory");
		return false;
	}
	m_Buffer = temp;
	m_Capacity = capacity;
	return true;
}
bool KVProtocol::Set(uint16_t key, int8_t val)
{
	return _Set(key, TYPE_I32, (int8_t*)&val, sizeof(int8_t));
}

bool KVProtocol::Set(uint16_t key, int16_t val)
{
	return _Set(key, TYPE_I32, (int8_t*)&val, sizeof(int16_t));
}

bool KVProtocol::Set(uint16_t key, int32_t val)
{
	return _Set(key, TYPE_I32, (int8_t*)&val, sizeof(int32_t));
}

bool KVProtocol::Set(uint16_t key, int64_t val)
{
	return _Set(key, TYPE_I64, (int8_t*)&val, sizeof(int64_t));
}

bool KVProtocol::Set(uint16_t key, int8_t *bytes, uint32_t size)
{
	if(bytes==NULL || size==0)
		return false;
	return _Set(key, TYPE_BYTES, bytes, size);
}

bool KVProtocol::Set(uint16_t key, string str)
{
	if(str.size() == 0)
		return false;
	return _Set(key, TYPE_BYTES, (int8_t*)str.c_str(), str.size());
}

bool KVProtocol::UnPack()
{
	m_PosMap.clear();
	int8_t *ptr_start = m_Buffer;
	int8_t *ptr_end   = m_Buffer+m_Size;
	while(ptr_start < ptr_end)
	{
		int8_t *pos = ptr_start;
		//key
		if(ptr_start+sizeof(uint32_t) > ptr_end)
			return false;
		uint16_t key_type = *(uint16_t*)ptr_start;
		ptr_start += sizeof(uint32_t);
		uint16_t key = ToKey(key_type);    //key
		uint16_t type = ToType(key_type);  //type
		if(type==TYPE_I8 || type==TYPE_I16)
			continue;
		else if(type == TYPE_I32)
		{
			ptr_start += sizeof(uint32_t);
		}
		else if(type == TYPE_I64)
		{
			ptr_start += sizeof(uint64_t);
		}
		else if(type==TYPE_BP0 || type==TYPE_BP1 || type==TYPE_BP2 || type==TYPE_BP3)
		{
			if(ptr_start+sizeof(uint32_t) > ptr_end)
				return false;
			uint32_t size = *(uint32_t)*ptr_start;
			ptr_start += sizeof(uint32_t);
			size += type-TYPE_BYTES;
			ptr_start += size;
		}
		else
			return false;

		PosMap::iterator it = m_PosMap.find(key);
		if(it == m_PosMap.end())
			m_PosMap.insert(std::make_pair(key, pos));
		else
			it->second = pos;
	}

	return true;
}

bool KVProtocol::Get(uint16_t key, int32_t *val)
{
	if(val == NULL)
		return false;
	return _Get(key, TYPE_I32, (int8_t**)val, NULL);
}

bool KVProtocol::Get(uint16_t key, int64_t *val)
{
	if(val == NULL)
		return false;
	return _Get(key, TYPE_I64, (int8_t**)val, NULL);
}

bool KVProtocol::Get(uint16_t key, int8_t **bytes, uint32_t *size)
{
	if(bytes == NULL || size==NULL)
		return false;
	return _Get(key, TYPE_BYTES, bytes, size);
}

bool KVProtocol::Get(uint16_t key, string *str)
{
	if(str == NULL)
		return false;
	int8_t *bytes = NULL;
	uint32_t size = 0;
	if(!_Get(key, TYPE_BYTES, &bytes, &size))
		return false;
	str->assign((const char*)bytes, size);
	return true;
}

bool KVProtocol::_Get(uint16_t key, uint16_t type, int8_t **bytes, uint32_t *size)
{
	PosMap::iterator it = m_PosMap.find(key);
	if(it == m_PosMap.end())
		return false;
	uint8_t *ptr = it->second;
	ptr += sizeof(uint16_t);
	uint16_t rel_type = *(uint16_t*)ptr;
	ptr += sizeof(uint16_t);
	if(rel_type >= TYPE_BP0)
	{
		rel_type = TYPE_BYTES;
		*size = *(uint32_t)ptr;
		ptr += sizeof(uint32_t);
	}

	if(type != rel_type)
		return false;
	if(type == TYPE_I32)
		memcpy((void*)bytes, ptr, sizeof(uint32_t));
	else if(type == TYPE_I64)
		memcpy((void*)bytes, ptr, sizeof(uint32_t));
	else
		*bytes = ptr;
	return true;
}

}//namespace
