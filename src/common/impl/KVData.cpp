/*
 * KVData.cpp
 *
 *  Created on: 2013-5-24
 *      Author: LiuYongJin
 */

#include "KVData.h"

#include <string.h>
#include <netinet/in.h>

namespace easynet
{
//最多只能有8个类型
#define TYPE_INT8      0
#define TYPE_INT16     1
#define TYPE_INT32     2
#define TYPE_INT64     3
#define TYPE_BYTES     4

#define KeyType(k, t)  (k<<3|t)  //key和type组合
#define ToKey(kt)      (kt>>3)   //转成key
#define ToType(kt)     (kt&0x07) //转成type

//for 64bit
#ifndef  htonll
#define  htonll(x)     (((uint64_t)(htonl((uint32_t)((x)&0xffffffff)))<<32) | htonl((uint32_t)(((x)>>32)&0xffffffff)))
#define  ntohll(x)     (((uint64_t)(ntohl((uint32_t)((x)&0xffffffff)))<<32) | ntohl((uint32_t)(((x)>>32)&0xffffffff)))
#endif

#define HEADER_SIZE      sizeof(uint16_t)
#define SIZE_INT8        HEADER_SIZE+sizeof(int8_t)
#define SIZE_INT16       HEADER_SIZE+sizeof(int16_t)
#define SIZE_INT32       HEADER_SIZE+sizeof(int32_t)
#define SIZE_INT64       HEADER_SIZE+sizeof(int64_t)
#define SIZE_BYTES(len)  HEADER_SIZE+sizeof(uint32_t)+len

#define CheckBuffer(buffer, len, esize) do{ \
if(buffer->Size+len > buffer->Capacity) \
{ \
	bool ret = buffer->Enlarge(esize); \
	assert(ret == true); \
}\
}while(0)

/////////////////////////////////// static method ///////////////////////////////////
uint32_t KVData::SizeInt(int8_t)
{
	return SIZE_INT8;
}

uint32_t KVData::SizeInt(uint8_t)
{
	return SIZE_INT8;
}

uint32_t KVData::SizeInt(int16_t)
{
	return SIZE_INT16;
}

uint32_t KVData::SizeInt(uint16_t)
{
	return SIZE_INT16;
}

uint32_t KVData::SizeInt(int32_t)
{
	return SIZE_INT32;
}

uint32_t KVData::SizeInt(uint32_t)
{
	return SIZE_INT32;
}

uint32_t KVData::SizeInt(int64_t)
{
	return SIZE_INT64;
}

uint32_t KVData::SizeInt(uint64_t)
{
	return SIZE_INT64;
}

uint32_t KVData::SizeBytes(uint32_t len)
{
	return SIZE_BYTES(len);
}

//int8
uint32_t KVData::SetValue(char *buffer, uint16_t key, int8_t value, bool net_trans/*=false*/)
{
	assert(buffer != NULL);
	uint16_t key_type = KeyType(key, TYPE_INT32);
	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	*(int8_t*)buffer = value;
	return SIZE_INT8;
}

uint32_t KVData::SetValue(char *buffer, uint16_t key, uint8_t value, bool net_trans/*=false*/)
{
	return SetValue(buffer, key, (int8_t)value, net_trans);
}

//int16
uint32_t KVData::SetValue(char *buffer, uint16_t key, int16_t  value, bool net_trans/*=false*/)
{
	assert(buffer != NULL);
	uint16_t key_type = KeyType(key, TYPE_INT32);
	if(net_trans)
	{
		key_type = htons(key_type);
		value = htons(value);
	}
	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	*(int16_t*)buffer = value;
	return SIZE_INT16;
}
uint32_t KVData::SetValue(char *buffer, uint16_t key, uint16_t value, bool net_trans/*=false*/)
{
	return SetValue(buffer, key, (int16_t)value, net_trans);
}

//int32
uint32_t KVData::SetValue(char *buffer, uint16_t key, int32_t  value, bool net_trans/*=false*/)
{
	assert(buffer != NULL);
	uint16_t key_type = KeyType(key, TYPE_INT32);
	if(net_trans)
	{
		key_type = htons(key_type);
		value = htonl(value);
	}
	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	*(int32_t*)buffer = value;
	return SIZE_INT32;
}
uint32_t KVData::SetValue(char *buffer, uint16_t key, uint32_t value, bool net_trans/*=false*/)
{
	return SetValue(buffer, key, (int32_t)value, net_trans);
}

//int64
uint32_t KVData::SetValue(char *buffer, uint16_t key, int64_t  value, bool net_trans/*=false*/)
{
	assert(buffer != NULL);
	uint16_t key_type = KeyType(key, TYPE_INT64);
	if(net_trans)
	{
		key_type = htons(key_type);
		value = htonll(value);
	}
	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	*(int64_t*)buffer = value;
	return SIZE_INT64;
}
uint32_t KVData::SetValue(char *buffer, uint16_t key, uint64_t value, bool net_trans/*=false*/)
{
	return SetValue(buffer, key, (int64_t)value, net_trans);
}

//bytes
uint32_t KVData::SetValue(char *buffer, uint16_t key, const string &str, bool net_trans/*=false*/)
{
	return SetValue(buffer, key, str.c_str(), str.size(), net_trans);
}
uint32_t KVData::SetValue(char *buffer, uint16_t key, const char *c_str, bool net_trans/*=false*/)
{
	assert(c_str != NULL);
	uint32_t len = strlen(c_str);
	assert(len > 0);
	return SetValue(buffer, key, c_str, len+1, net_trans);
}
uint32_t KVData::SetValue(char *buffer, uint16_t key, const char *data, uint32_t len, bool net_trans/*=false*/)
{
	assert(buffer!=NULL && data!=NULL && len>0);
	uint16_t key_type = KeyType(key, TYPE_BYTES);
	uint32_t temp_size = len;
	if(net_trans)
	{
		key_type = htons(key_type);
		temp_size = htonl(len);
	}

	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	*(uint32_t*)buffer = temp_size;
	buffer += sizeof(uint32_t);
	memcpy(buffer, data, len);
	buffer += len;
	return SIZE_BYTES(len);
}
/////////////////////////////////// end static method ///////////////////////////////////

#define SAVE_ITEM(item) KVItemMap::iterator it = m_ItemMap.find(key); \
assert(it == m_ItemMap.end());  \
std::pair<KVItemMap::iterator, bool> ret;  \
ret = m_ItemMap.insert(std::make_pair(key, item)); \
assert(ret.second == true);

//int8
void KVData::SetValue(uint16_t key, int8_t   value)
{
	KVItem item;
	item.key = key;
	item.type = (uint8_t)TYPE_INT8;
	item.value.value_i8 = value;
	SAVE_ITEM(item);
	m_Size += SIZE_INT8;
}
void KVData::SetValue(uint16_t key, uint8_t  value)
{
	KVItem item;
	item.key = key;
	item.type = (uint8_t)TYPE_INT8;
	item.value.value_i8 = (int8_t)value;
	SAVE_ITEM(item);
	m_Size += SIZE_INT8;
}

//int16
void KVData::SetValue(uint16_t key, int16_t  value)
{
	KVItem item;
	item.key = key;
	item.type = (uint8_t)TYPE_INT16;
	item.value.value_i16 = value;
	SAVE_ITEM(item);
	m_Size += SIZE_INT16;
}
void KVData::SetValue(uint16_t key, uint16_t value)
{
	KVItem item;
	item.key = key;
	item.type = (uint8_t)TYPE_INT16;
	item.value.value_i16 = (int16_t)value;
	SAVE_ITEM(item);
	m_Size += SIZE_INT16;
}

//int32
void KVData::SetValue(uint16_t key, int32_t  value)
{
	KVItem item;
	item.key = key;
	item.type = (uint8_t)TYPE_INT32;
	item.value.value_i32 = value;
	SAVE_ITEM(item);
	m_Size += SIZE_INT32;
}
void KVData::SetValue(uint16_t key, uint32_t value)
{
	KVItem item;
	item.key = key;
	item.type = (uint8_t)TYPE_INT32;
	item.value.value_i32 = (int32_t)value;
	SAVE_ITEM(item);
	m_Size += SIZE_INT32;
}

//int64
void KVData::SetValue(uint16_t key, int64_t  value)
{
	KVItem item;
	item.key = key;
	item.type = (uint8_t)TYPE_INT64;
	item.value.value_i64 = value;
	SAVE_ITEM(item);
	m_Size += SIZE_INT64;
}
void KVData::SetValue(uint16_t key, uint64_t value)
{
	KVItem item;
	item.key = key;
	item.type = (uint8_t)TYPE_INT64;
	item.value.value_i64 = (int64_t)value;
	SAVE_ITEM(item);
	m_Size += SIZE_INT64;
}

//bytes
void KVData::SetValue(uint16_t key, const char *data, uint32_t len)
{
	assert(data!=NULL && len>0);
	KVItem item;
	item.key = key;
	item.type = (uint8_t)TYPE_BYTES;
	item.value.value_len = len;
	item.value_bytes = (char*)data;
	SAVE_ITEM(item);
	m_Size += SIZE_BYTES(len);
}
void KVData::SetValue(uint16_t key, const string &str)
{
	assert(str.size() > 0);
	SetValue(key, str.c_str(), (uint32_t)str.size());
}
void KVData::SetValue(uint16_t key, const char *c_str)
{
	assert(c_str != NULL);
	uint32_t len = strlen(c_str)+1;
	assert(len > 1);
	SetValue(key, c_str, len);
}

////////////////////////////////////
void KVData::Clear()
{
	m_ItemMap.clear();
	m_Size = 0;
}

uint32_t KVData::Serialize(char *buffer)
{
	KVItemMap::iterator it;
	char *src = buffer;
	for(it=m_ItemMap.begin(); it!=m_ItemMap.end(); ++it)
	{
		KVItem &item = it->second;
		switch(item.type)
		{
		case TYPE_INT8:
			buffer += SetValue(buffer, item.key, item.value.value_i8, m_NetTrans);
			break;
		case TYPE_INT16:
			buffer += SetValue(buffer, item.key, item.value.value_i16, m_NetTrans);
			break;
		case TYPE_INT32:
			buffer += SetValue(buffer, item.key, item.value.value_i32, m_NetTrans);
			break;
		case TYPE_INT64:
			buffer += SetValue(buffer, item.key, item.value.value_i64, m_NetTrans);
			break;
		case TYPE_BYTES:
			buffer += SetValue(buffer, item.key, item.value_bytes, item.value.value_len, m_NetTrans);
			break;
		default:
			assert(0);
			break;
		}
	}
	assert(src+m_Size == buffer);
	return m_Size;
}


#define CHECK_TYPE_SIZE(type) if(ptr+sizeof(type) > BUFFER_END) \
{ \
result = false; \
break; \
}
//反序列化,成功返回true,失败返回false
bool KVData::UnSerialize(KVItemMap &item_map, const char *buffer, uint32_t size, bool net_trans/*=false*/)
{
	assert(buffer != NULL);
	item_map.clear();

	bool result = true;
	char *ptr = (char*)buffer;
	const char*BUFFER_END = buffer+size;
	while(ptr < BUFFER_END)
	{
		if(ptr+HEADER_SIZE > BUFFER_END)
		{
			result = false;
			break;
		}

		//1. key_type
		uint16_t key_type = *(uint16_t*)ptr;
		ptr += HEADER_SIZE;

		if(net_trans)
			key_type = ntohs(key_type);
		KVItem item;
		item.key = ToKey(key_type);
		item.type = ToType(key_type);

		if(item.type == TYPE_INT8)
		{
			CHECK_TYPE_SIZE(int8_t);
			item.value.value_i8 = *(int8_t*)ptr;
			ptr += sizeof(int8_t);
		}
		else if(item.type == TYPE_INT16)
		{
			CHECK_TYPE_SIZE(int16_t);
			item.value.value_i16 = *(int16_t*)ptr;
			if(net_trans)
				item.value.value_i16 = ntohs(item.value.value_i16);
			ptr += sizeof(int16_t);
		}
		else if(item.type == TYPE_INT32)
		{
			CHECK_TYPE_SIZE(int32_t);
			item.value.value_i32 = *(int32_t*)ptr;
			if(net_trans)
				item.value.value_i32 = ntohl(item.value.value_i32);
			ptr += sizeof(int32_t);
		}
		else if(item.type == TYPE_INT64)
		{
			CHECK_TYPE_SIZE(int64_t);
			item.value.value_i64 = *(int64_t*)ptr;
			if(net_trans)
				item.value.value_i64 = ntohll(item.value.value_i64);
			ptr += sizeof(int64_t);
		}
		else if(item.type == TYPE_BYTES)
		{
			CHECK_TYPE_SIZE(uint32_t);
			item.value.value_len = *(uint32_t*)ptr;
			if(net_trans)
				item.value.value_len = ntohl(item.value.value_len);
			ptr += sizeof(uint32_t);
			if(ptr+item.value.value_len > BUFFER_END)
			{
				result = false;
				break;
			}
			item.value_bytes = ptr;
			ptr += item.value.value_len;
		}
		else
		{
			result = false;
			break;
		}

		//insert into map
		KVItemMap::iterator it = item_map.find(item.key);
		if(it != item_map.end())
			it->second = item;
		else
			item_map.insert(std::make_pair(item.key, item));
	}

	if(result == false)
		item_map.clear();
	return result;
}

#define GET_ITEM(key, t) KVItemMap::iterator it = item_map.find(key); \
if(it == item_map.end())  \
	return false;  \
KVItem &item = it->second;  \
if(item.type != t)  \
	return false;

//int8
bool KVData::GetValue(KVItemMap &item_map, uint16_t key, int8_t   &value)
{
	GET_ITEM(key, TYPE_INT8);
	value = item.value.value_i8;
	return true;
}
bool KVData::GetValue(KVItemMap &item_map, uint16_t key, uint8_t  &value)
{
	GET_ITEM(key, TYPE_INT8);
	value = (uint8_t)item.value.value_i8;
	return true;
}

//int16
bool KVData::GetValue(KVItemMap &item_map, uint16_t key, int16_t  &value)
{
	GET_ITEM(key, TYPE_INT16);
	value = item.value.value_i16;
	return true;
}
bool KVData::GetValue(KVItemMap &item_map, uint16_t key, uint16_t &value)
{
	GET_ITEM(key, TYPE_INT16);
	value = (uint16_t)item.value.value_i16;
	return true;
}

//int32
bool KVData::GetValue(KVItemMap &item_map, uint16_t key, int32_t  &value)
{
	GET_ITEM(key, TYPE_INT32);
	value = item.value.value_i32;
	return true;
}
bool KVData::GetValue(KVItemMap &item_map, uint16_t key, uint32_t &value)
{
	GET_ITEM(key, TYPE_INT32);
	value = (uint32_t)item.value.value_i32;
	return true;
}

bool KVData::GetValue(KVItemMap &item_map, uint16_t key, int64_t  &value)
{
	GET_ITEM(key, TYPE_INT64);
	value = item.value.value_i64;
	return true;
}

bool KVData::GetValue(KVItemMap &item_map, uint16_t key, uint64_t &value)
{
	GET_ITEM(key, TYPE_INT64);
	value = (uint64_t)item.value.value_i64;
	return true;
}

bool KVData::GetValue(char *buffer, uint16_t key, char *&data, uint32_t &len)
{
	GET_ITEM(key, TYPE_BYTES);
	len = item.value.value_len;
	data = item.value_bytes;
	return true;
}

bool KVData::GetValue(char *buffer, uint16_t key, string &str)
{
	GET_ITEM(key, TYPE_BYTES);
	str.assign(item.value_bytes, item.value.value_len);
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
void KVData::SetInt32(uint16_t key, int32_t value, char *&buffer, bool net_trans)
{
	assert(buffer != NULL);

	uint16_t key_type = KeyType(key, TYPE_INT32);
	if(net_trans)
	{
		key_type = htons(key_type);
		value = htonl(value);
	}

	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	*(int32_t*)buffer = value;
	buffer += sizeof(int32_t);
}

void KVData::SetInt32(uint16_t key, uint32_t value, char *&buffer, bool net_trans)
{
	SetInt32(key, (int32_t)value, buffer, net_trans);
}

void KVData::SetInt64(uint16_t key, int64_t value, char *&buffer, bool net_trans)
{
	assert(buffer != NULL);

	uint16_t key_type = KeyType(key, TYPE_INT64);
	if(net_trans)
	{
		key_type = htons(key_type);
		value = htonll(value);
	}

	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	*(int64_t*)buffer = value;
	buffer += sizeof(int64_t);
}

void KVData::SetInt64(uint16_t key, uint64_t value, char *&buffer, bool net_trans)
{
	SetInt64(key, (int64_t)value, buffer, net_trans);
}

void KVData::SetBytes(uint16_t key, const string &str, char *&buffer, bool net_trans)
{
	SetBytes(key, str.c_str(), str.size(), buffer, net_trans);
}

void KVData::SetBytes(uint16_t key, const char *data, uint32_t size, char *&buffer, bool net_trans)
{
	assert(buffer != NULL);
	uint16_t key_type = KeyType(key, TYPE_BYTES);
	uint32_t temp_size = size;
	if(net_trans)
	{
		key_type = htons(key_type);
		temp_size = htonl(size);
	}

	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	*(uint32_t*)buffer = temp_size;
	buffer += sizeof(uint32_t);
	memcpy(buffer, data, size);
	buffer += size;
}

char* KVData::GetWriteBuffer(uint16_t key, char *buffer, bool net_trans)
{
	assert(buffer != NULL);

	uint16_t key_type = KeyType(key, TYPE_BYTES);
	if(net_trans)
		key_type = htons(key_type);
	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	return buffer;
}

//调用者往缓冲区写入数据后,调用本方法,设置实际写入的数据大小.key和GetWriteBuffer中设置的key必须一致.
//  如果没调用本方法,则相当于之前调用GetDataBuffer和写入缓冲区的数据无效.
//  不允许调用本方法之前调用其他SetValue方法.
void KVData::SetWriteLength(uint16_t key, uint32_t size, char *&buffer, bool net_trans)
{
	assert(buffer != NULL);

	//检验是否是之前写入的头部信息
	uint16_t key_type = *(uint16_t*)buffer;
	if(net_trans)
	{
		key_type = ntohs(key_type);
		size = htonl(size);
	}
	uint16_t cur_key = ToKey(key_type);
	uint16_t cur_type = ToType(key_type);
	assert(cur_key==key && cur_type==TYPE_BYTES);

	//写入实际的长度
	buffer += sizeof(uint16_t);
	*(uint32_t*)buffer = size;
	buffer += sizeof(uint32_t)+size;
}

void KVData::SetInt32(uint16_t key, int32_t value, ByteBuffer *buffer, bool net_trans)
{
	assert(buffer != NULL);
	uint32_t len = sizeof(uint16_t)+sizeof(int32_t);
	CheckBuffer(buffer, len, 128);

	uint16_t key_type = KeyType(key, TYPE_INT32);
	if(net_trans)
	{
		key_type = htons(key_type);
		value = htonl(value);
	}

	char *ptr = buffer->Buffer+buffer->Size;
	*(uint16_t*)ptr = key_type;
	ptr += sizeof(uint16_t);
	*(int32_t*)ptr = value;

	buffer->Size += len;
}
void KVData::SetInt32(uint16_t key, uint32_t value, ByteBuffer *buffer, bool net_trans)
{
	SetInt32(key, (int32_t)value, buffer, net_trans);
}

void KVData::SetInt64(uint16_t key, int64_t value, ByteBuffer *buffer, bool net_trans)
{
	assert(buffer != NULL);
	uint32_t len = sizeof(uint16_t)+sizeof(int64_t);
	CheckBuffer(buffer, len, 128);

	uint16_t key_type = KeyType(key, TYPE_INT64);
	if(net_trans)
	{
		key_type = htons(key_type);
		value = htonll(value);
	}

	char *ptr = buffer->Buffer+buffer->Size;
	*(uint16_t*)ptr = key_type;
	ptr += sizeof(uint16_t);
	*(int64_t*)ptr = value;

	buffer->Size += len;
}

void KVData::SetInt64(uint16_t key, uint64_t value, ByteBuffer *buffer, bool net_trans)
{
	SetInt64(key, (int64_t)value, buffer, net_trans);
}

void KVData::SetBytes(uint16_t key, const string &str, ByteBuffer *buffer, bool net_trans)
{
	SetBytes(key, str.c_str(), str.size(), buffer, net_trans);
}

void KVData::SetBytes(uint16_t key, const char *data, uint32_t size, ByteBuffer *buffer, bool net_trans)
{
	assert(buffer!=NULL && data!=NULL);
	uint32_t len = sizeof(uint16_t)+sizeof(uint32_t)+size;
	CheckBuffer(buffer, len, len+128);

	uint16_t key_type = KeyType(key, TYPE_BYTES);
	uint32_t temp_size = size;
	if(net_trans)
	{
		key_type = htons(key_type);
		temp_size = htonl(size);
	}

	char *ptr = buffer->Buffer+buffer->Size;
	*(uint16_t*)ptr = key_type;
	ptr += sizeof(uint16_t);
	*(uint32_t*)ptr = temp_size;
	ptr += sizeof(uint32_t);
	memcpy(ptr, data, size);

	buffer->Size += len;
}

char* KVData::GetWriteBuffer(uint16_t key, uint32_t max_size, ByteBuffer *buffer, bool net_trans)
{
	assert(buffer != NULL);
	uint32_t len = sizeof(uint16_t)+sizeof(uint32_t)+max_size;
	CheckBuffer(buffer, len, len+128);

	uint16_t key_type = KeyType(key, TYPE_BYTES);
	if(net_trans)
		key_type = htons(key_type);
	char *ptr = buffer->Buffer+buffer->Size;
	*(uint16_t*)ptr = key_type;
	ptr += sizeof(uint16_t)+sizeof(uint32_t);
	return ptr;
}

void KVData::SetWriteLength(uint16_t key, uint32_t size, ByteBuffer *buffer, bool net_trans)
{
	assert(buffer != NULL);
	uint32_t len = sizeof(uint16_t)+sizeof(uint32_t)+size;

	//检验是否是之前写入的头部信息
	char *ptr = buffer->Buffer+buffer->Size;
	uint16_t key_type = *(uint16_t*)ptr;
	if(net_trans)
	{
		key_type = ntohs(key_type);
		size = htonl(size);
	}
	uint16_t cur_key = ToKey(key_type);
	uint16_t cur_type = ToType(key_type);
	assert(cur_key==key && cur_type==TYPE_BYTES && buffer->Size+len<=buffer->Capacity);

	//写入实际的长度
	ptr += sizeof(uint16_t);
	*(uint32_t*)ptr = size;
	buffer->Size += len;
}

bool KVData::UnPack(KVItemMap &item_map, const char *buffer, uint32_t size, bool net_trans)
{
	assert(buffer != NULL);
	item_map.clear();

	bool result = true;
	char *ptr = (char*)buffer;
	const char*BUFFER_END = buffer+size;
	while(ptr < BUFFER_END)
	{
		if(ptr + sizeof(uint16_t) > BUFFER_END)
		{
			result = false;
			break;
		}

		//1. key_type
		uint16_t key_type = *(uint16_t*)ptr;
		ptr += sizeof(uint16_t);

		if(net_trans)
			key_type = ntohs(key_type);
		KVItem item;
		item.key = ToKey(key_type);
		item.type = ToType(key_type);
		if(item.type == TYPE_INT32)
		{
			if(ptr+sizeof(int32_t) > BUFFER_END)
			{
				result = false;
				break;
			}

			item.value.value_i32 = *(int32_t*)ptr;
			if(net_trans)
				item.value.value_i32 = ntohl(item.value.value_i32);
			ptr += sizeof(int32_t);
		}
		else if(item.type == TYPE_INT64)
		{
			if(ptr+sizeof(int64_t) > BUFFER_END)
			{
				result = false;
				break;
			}

			item.value.value_i64 = *(int64_t*)ptr;
			if(net_trans)
				item.value.value_i64 = ntohll(item.value.value_i64);
			ptr += sizeof(int64_t);
		}
		else if(item.type == TYPE_BYTES)
		{
			if(ptr+sizeof(uint32_t) > BUFFER_END)
			{
				result = false;
				break;
			}

			item.value.length = *(uint32_t*)ptr;
			if(net_trans)
				item.value.length = ntohl(item.value.length);
			ptr += sizeof(uint32_t);
			if(ptr+item.value.length > BUFFER_END)
			{
				result = false;
				break;
			}
			item.value_bytes = ptr;
			ptr += item.value.length;
		}
		else    //unknow type
		{
			result = false;
			break;
		}

		//insert into map
		KVItemMap::iterator it = item_map.find(item.key);
		if(it != item_map.end())
			it->second = item;
		else
			item_map.insert(std::make_pair(item.key, item));
	}

	if(result == false)
		item_map.clear();
	return result;
}

bool KVData::GetInt32(KVItemMap &item_map, uint16_t key, int32_t &value)
{
	KVItemMap::iterator it = item_map.find(key);
	if(it == item_map.end())
		return false;
	KVItem &item = it->second;
	if(item.type != TYPE_INT32)
		return false;
	value = item.value.value_i32;
	return true;
}

bool KVData::GetInt32(KVItemMap &item_map, uint16_t key, uint32_t &value)
{
	int32_t temp;
	bool ret = GetInt32(item_map, key, temp);
	if(ret == true)
		value = (uint32_t)temp;
	return ret;
}

bool KVData::GetInt64(KVItemMap &item_map, uint16_t key, int64_t &value)
{
	KVItemMap::iterator it = item_map.find(key);
	if(it == item_map.end())
		return false;
	KVItem &item = it->second;
	if(item.type != TYPE_INT64)
		return false;
	value = item.value.value_i64;
	return true;
}
bool KVData::GetInt64(KVItemMap &item_map, uint16_t key, uint64_t &value)
{
	int64_t temp;
	bool ret = GetInt64(item_map, key, temp);
	if(ret == true)
		value = (uint64_t)temp;
	return ret;
}

bool KVData::GetBytes(KVItemMap &item_map, uint16_t key, string &str)
{
	KVItemMap::iterator it = item_map.find(key);
	if(it == item_map.end())
		return false;
	KVItem &item = it->second;
	if(item.type != TYPE_BYTES)
		return false;
	str.assign(item.value_bytes, item.value.length);
	return true;
}

bool KVData::GetBytes(KVItemMap &item_map, uint16_t key, char *&data, uint32_t &size)
{
	KVItemMap::iterator it = item_map.find(key);
	if(it == item_map.end())
		return false;
	KVItem &item = it->second;
	if(item.type != TYPE_BYTES)
		return false;
	size = item.value.length;
	data = item.value_bytes;
	return true;
}

}//easynet
