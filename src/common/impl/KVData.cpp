/*
 * KVData.cpp
 *
 *  Created on: 2013-5-24
 *      Author: LiuYongJin
 */

#include "KVData.h"
#include <stdio.h>
#include <stdlib.h>
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
#define TYPE_KVDATA    5

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
#define SIZE_BYTES(len)  HEADER_SIZE+sizeof(uint32_t)+(len)

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
	uint16_t key_type = KeyType(key, TYPE_INT8);
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
	uint16_t key_type = KeyType(key, TYPE_INT16);
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
		else if(item.type==TYPE_BYTES || item.type==TYPE_KVDATA)
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

bool KVData::GetValue(KVItemMap &item_map, uint16_t key, char *&data, uint32_t &len)
{
	GET_ITEM(key, TYPE_BYTES);
	len = item.value.value_len;
	data = item.value_bytes;
	return true;
}

bool KVData::GetValue(KVItemMap &item_map, uint16_t key, string &str)
{
	GET_ITEM(key, TYPE_BYTES);
	str.assign(item.value_bytes, item.value.value_len);
	return true;
}

bool KVData::GetValue(KVItemMap &item_map, uint16_t key, KVData &kv_data)
{
	GET_ITEM(key, TYPE_KVDATA);
	kv_data.Clear();
	return kv_data.UnSerialize(item.value_bytes, item.value.value_len);
}

//开始写数据
//  返回值:pair的second为可写数据的缓冲区
KVBuffer KVData::BeginWrite(char *buffer, uint16_t key, bool net_trans/*=false*/)
{
	assert(buffer != NULL);
	KVBuffer raw_buf;
	raw_buf.first = buffer;
	raw_buf.second = buffer+HEADER_SIZE+sizeof(uint32_t);
	raw_buf.key = key;
	raw_buf.net_trans = net_trans;
	return raw_buf;
}

uint32_t KVData::EndWrite(KVBuffer &raw_buf, uint32_t len)
{
	return EndWrite(raw_buf, len, TYPE_BYTES);
}

//写数据结束
//  buf_pair:的BeginWrite返回值
//  len:实际写入的字节数
//private
uint32_t KVData::EndWrite(KVBuffer &raw_buf, uint32_t len, uint8_t type)
{
	assert(raw_buf.first!=NULL && raw_buf.second==raw_buf.first+HEADER_SIZE+sizeof(uint32_t));
	if(len == 0)
		return 0;
	char *buffer = (char*)raw_buf.first;
	uint16_t key_type = KeyType(raw_buf.key, type);
	uint32_t temp_size = len;
	if(raw_buf.net_trans)
	{
		key_type = htons(key_type);
		temp_size = htonl(len);
	}
	*(uint16_t*)buffer = key_type;  //key
	buffer += sizeof(uint16_t);
	*(uint32_t*)buffer = temp_size; //len
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

void KVData::SetValue(uint16_t key, KVData *value)
{
	assert(value!=NULL && value->Size()>0);
	map<uint16_t, KVData*>::iterator it = m_KVMap.find(key);
	assert(it == m_KVMap.end());

	std::pair<map<uint16_t, KVData*>::iterator, bool> ret;
	ret = m_KVMap.insert(std::make_pair(key, value));
	assert(ret.second == true);
	m_Size += SIZE_BYTES(value->Size());  //数据头长度+数据长度
}

////////////////////////////////////
void KVData::Clear()
{
	m_ItemMap.clear();
	m_KVMap.clear();
	m_Size = 0;
}

uint32_t KVData::Size()
{
	return m_Size;
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

	//序列化KVData
	map<uint16_t, KVData*>::iterator kv_it;
	for(kv_it = m_KVMap.begin(); kv_it!=m_KVMap.end(); ++kv_it)
	{
		KVData *kv_value = kv_it->second;
		assert(kv_value != NULL);
		KVBuffer kv_buffer = BeginWrite(buffer, kv_it->first, m_NetTrans);
		kv_value->Serialize(kv_buffer.second);
		buffer += EndWrite(kv_buffer, kv_value->Size(), (uint8_t)TYPE_KVDATA);
	}

	assert(src+m_Size == buffer);
	return m_Size;
}

bool KVData::UnSerialize(const char *buffer, uint32_t size)
{
	m_Size = size;
	return UnSerialize(m_ItemMap, buffer, size, m_NetTrans);
}

//int8
bool KVData::GetValue(uint16_t key, int8_t   &value)
{
	return GetValue(m_ItemMap, key, value);
}
bool KVData::GetValue(uint16_t key, uint8_t  &value)
{
	return GetValue(m_ItemMap, key, value);
}

//int16
bool KVData::GetValue(uint16_t key, int16_t  &value)
{
	return GetValue(m_ItemMap, key, value);
}
bool KVData::GetValue(uint16_t key, uint16_t &value)
{
	return GetValue(m_ItemMap, key, value);
}

//int32
bool KVData::GetValue(uint16_t key, int32_t  &value)
{
	return GetValue(m_ItemMap, key, value);
}
bool KVData::GetValue(uint16_t key, uint32_t &value)
{
	return GetValue(m_ItemMap, key, value);
}

//int64
bool KVData::GetValue(uint16_t key, int64_t  &value)
{
	return GetValue(m_ItemMap, key, value);
}
bool KVData::GetValue(uint16_t key, uint64_t &value)
{
	return GetValue(m_ItemMap, key, value);
}

//bytes
bool KVData::GetValue(uint16_t key, char *&data, uint32_t &len)
{
	return GetValue(m_ItemMap, key, data, len);
}

bool KVData::GetValue(uint16_t key, string &str)
{
	return GetValue(m_ItemMap, key, str);
}

//kvdata
bool KVData::GetValue(uint16_t key, KVData &kv_data)
{
	return GetValue(m_ItemMap, key, kv_data);
}

bool KVData::UnSerializeFromFile(const char *file_data, uint32_t len)
{
	if(file_data==NULL || len<8)
		return false;
	if(file_data[0]!='k' || file_data[1]!='v' || file_data[2]!='d'|| (file_data[3]!=1&&file_data[3]!=0))
		return false;

	Clear();
	m_NetTrans = (file_data[3]==0?false:true);
	m_Size = *(uint32_t*)(file_data+4);
	m_Size = ntohl(m_Size);
	if(m_Size+8 > len)
		return false;

	return UnSerialize(file_data+8, m_Size);
}

bool KVData::SerializeToFile(const char *file)
{
	if(file == NULL)
		return false;
	FILE *fp = fopen(file, "wb");
	if(fp == NULL)
		return false;
	char buffer[8];
	buffer[0] = 'k';
	buffer[1] = 'v';
	buffer[2] = 'd';
	buffer[3] = m_NetTrans?1:0;

	uint32_t size = m_Size;
	size = htonl(size);
	*(uint32_t*)(buffer+4) = size;
	fwrite(buffer, 1, 8, fp);

	if(m_Size > 0)
	{
		char *data = (char*)malloc(m_Size);
		Serialize(data);
		fwrite(data, 1, m_Size, fp);
	}

	fclose(fp);
	return 0;
}

}//easynet
