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

#define TYPE_INT32     0
#define TYPE_INT64     1
#define TYPE_BYTES     2

#define KeyType(k, t)  (k<<3|t)
#define ToKey(ky)      (ky>>3)
#define ToType(ky)     (ky&0x07)


//for 64bit
#ifndef  htonll
#define  htonll(x)     (((uint64_t)(htonl((uint32_t)((x)&0xffffffff)))<<32) | htonl((uint32_t)(((x)>>32)&0xffffffff)))
#define  ntohll(x)     (((uint64_t)(ntohl((uint32_t)((x)&0xffffffff)))<<32) | ntohl((uint32_t)(((x)>>32)&0xffffffff)))
#endif


#define CheckBuffer(buffer, len, esize) do{ \
if(buffer->Size+len > buffer->Capacity) \
{ \
	bool ret = buffer->Enlarge(esize); \
	assert(ret == true); \
}\
}while(0)


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

void KVData::SetInt64(uint16_t key, int64_t value, char *&buffer, bool net_trans)
{
	assert(buffer != NULL);

	uint16_t key_type = KeyType(key, TYPE_INT64);
	if(net_trans)
	{
		key_type = htons(key_type);
		value = htonl(value);
	}

	*(uint16_t*)buffer = key_type;
	buffer += sizeof(uint16_t);
	*(int64_t*)buffer = value;
	buffer += sizeof(int64_t);
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
