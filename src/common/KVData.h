/*
 * KVData.h
 *
 *  Created on: 2013-5-24
 *      Author: LiuYongJin
 */

#ifndef _COMMON_KVDATA_H_
#define _COMMON_KVDATA_H_

#include <assert.h>
#include <stdint.h>
#include <map>
#include <string>
using std::map;
using std::string;

#include "ByteBuffer.h"

namespace easynet
{

typedef struct _kv_item_
{
	uint16_t key;
	uint8_t type;
	union
	{
		int8_t   value_i8;
		int16_t  value_i16;
		int32_t  value_i32;
		int64_t  value_i64;
		uint32_t value_len;
	}value;
	char *value_bytes;
}KVItem;

#define KV_INVALID_POS -1
typedef uint32_t KVItemPos;
typedef map<uint16_t, KVItem> KVItemMap;

class KVData
{
public:
	////////////////////////////////////////////////////////////////////
	//// 打包                                                       ////
	////   将key-value值写入buffer中                                ////
	////   net_trans为true时会调用htons等函数进行转换               ////
	////   值对占用的字节数:头部大小和值占用字节数                  ////
	////   调用者需要保存buffer有足够空间                           ////
	////////////////////////////////////////////////////////////////////

	//存放数据需要占用的字节数
	static uint32_t SizeInt(int8_t);
	static uint32_t SizeInt(uint8_t);
	static uint32_t SizeInt(int16_t);
	static uint32_t SizeInt(uint16_t);
	static uint32_t SizeInt(int32_t);
	static uint32_t SizeInt(uint32_t);
	static uint32_t SizeInt(int64_t);
	static uint32_t SizeInt(uint64_t);
	static uint32_t SizeBytes(uint32_t len);

	//设置key-value值对
	//返回值:值对占用的字节数
	static uint32_t SetValue(char *buffer, uint16_t key, int8_t   value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, uint8_t  value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, int16_t  value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, uint16_t value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, int32_t  value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, uint32_t value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, int64_t  value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, uint64_t value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, const string &str, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, const char *c_str, bool net_trans=false);  //包含'\0'
	static uint32_t SetValue(char *buffer, uint16_t key, const char *data, uint32_t len, bool net_trans=false);

	//反序列化,成功返回true,失败返回false
	static bool UnSerialize(KVItemMap &item_map, const char *buffer, uint32_t size, bool net_trans=false);

	//获取key对应的值,成功返回true,失败返回false
	static bool GetValue(KVItemMap &item_map, uint16_t key, int8_t   &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, uint8_t  &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, int16_t  &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, uint16_t &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, int32_t  &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, uint32_t &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, int64_t  &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, uint64_t &value);

	static bool GetValue(char *buffer, uint16_t key, char *&data, uint32_t &len);
	static bool GetValue(char *buffer, uint16_t key, string &str);

public:
	KVData():m_NetTrans(false), m_Size(0){}
	KVData(bool net_trans):m_NetTrans(net_trans), m_Size(0){}

	void Clear();    //清空数据
	uint32_t Size();  //保存值对所需要的字节数

	//序列化
	//  将值对保存到buffer中,buffer的大小不能小于Size();
	//  返回值:返回序列化后数据大小,和Size()的值应该相等;
	uint32_t Serialize(char *buffer);

	void SetValue(uint16_t key, int8_t   value);
	void SetValue(uint16_t key, uint8_t  value);
	void SetValue(uint16_t key, int16_t  value);
	void SetValue(uint16_t key, uint16_t value);
	void SetValue(uint16_t key, int32_t  value);
	void SetValue(uint16_t key, uint32_t value);
	void SetValue(uint16_t key, int64_t  value);
	void SetValue(uint16_t key, uint64_t value);
	void SetValue(uint16_t key, const string &str);
	void SetValue(uint16_t key, const char *c_str);  //包含'\0'
	void SetValue(uint16_t key, const char *data, uint32_t len);
public:
	//////////////////////////////////////////////////////////////////////
	////                             打包                             ////
	//// 往buffer中写入key-value值对.                                 ////
	//// net_trans为true时会调用htons等函数进行转换.                  ////
	//////////////////////////////////////////////////////////////////////
	//将数据写入buffer,并设置buffer为下一次写入的地址(需要保证buffer足够大)
	static void SetInt32(uint16_t key, int32_t value, char *&buffer, bool net_trans);
	static void SetInt32(uint16_t key, uint32_t value, char *&buffer, bool net_trans);
	static void SetInt64(uint16_t key, int64_t value, char *&buffer, bool net_trans);
	static void SetInt64(uint16_t key, uint64_t value, char *&buffer, bool net_trans);
	static void SetBytes(uint16_t key, const string &str, char *&buffer, bool net_trans);
	static void SetBytes(uint16_t key, const char *data, uint32_t size, char *&buffer, bool net_trans);

	//GetDataBuffer和SetDataLength两个方法成对调用,作用相当于SetBytes方法.用来写入数据块(当数据量比较大的时候,可减少数据的拷贝次数,提高性能).
	//获取字符串缓冲区,调用者可以直接往缓冲区写数据
	static char *GetWriteBuffer(uint16_t key, char *buffer, bool net_trans);

	//调用者往缓冲区写入数据后,调用本方法,设置实际写入的数据大小.key和GetWriteBuffer中设置的key必须一致.
	//  如果没调用本方法,则相当于之前调用GetDataBuffer和写入缓冲区的数据无效.
	//  不允许调用本方法之前调用其他SetValue方法.
	//buffer为GetWriteBuffer的参数buffer,而不是其返回的可写缓冲区.同时设置buffer为下一次写入的地址.
	static void SetWriteLength(uint16_t key, uint32_t size, char *&buffer, bool net_trans);

	//将数据写入bytebuffer,并设置bytebuffer的大小,下次从bytebuffer的数据后面写入
	static void SetInt32(uint16_t key, int32_t value, ByteBuffer *buffer, bool net_trans);
	static void SetInt32(uint16_t key, uint32_t value, ByteBuffer *buffer, bool net_trans);
	static void SetInt64(uint16_t key, int64_t value, ByteBuffer *buffer, bool net_trans);
	static void SetInt64(uint16_t key, uint64_t value, ByteBuffer *buffer, bool net_trans);
	static void SetBytes(uint16_t key, const string &str, ByteBuffer *buffer, bool net_trans);
	static void SetBytes(uint16_t key, const char *data, uint32_t size, ByteBuffer *buffer, bool net_trans);

	//GetDataBuffer和SetDataLength两个方法成对调用,用来写入数据块(当数据量比较大的时候,可减少数据的拷贝次数,提高性能).
	//获取字符串缓冲区,调用者可以直接往缓冲区写数据
	static char *GetWriteBuffer(uint16_t key, uint32_t max_size, ByteBuffer *buffer, bool net_trans);
	//调用者往缓冲区写入数据后,调用本方法,设置实际写入的数据大小.key和GetWriteBuffer中设置的key必须一致.
	//  如果没调用本方法,则相当于之前调用GetDataBuffer和写入缓冲区的数据无效.
	//  不允许调用SetBytes本方法之前调用其他SetValue方法.
	static void SetWriteLength(uint16_t key, uint32_t size, ByteBuffer *buffer, bool net_trans);

	//////////////////////////////////////////////////////////////////////
	////////////////////////         解包         ////////////////////////
	//////////////////////////////////////////////////////////////////////
	//对buffer进行解包,获取key-value值对,解包后的key-value保存到item_map中.
	static bool UnPack(KVItemMap &item_map, const char *buffer, uint32_t size, bool net_trans);
	//从item-map中获取key对应的value.必需在Unpack成功之后才能调用GetValue方法.
	static bool GetInt32(KVItemMap &item_map, uint16_t key, int32_t &value);
	static bool GetInt32(KVItemMap &item_map, uint16_t key, uint32_t &value);
	static bool GetInt64(KVItemMap &item_map, uint16_t key, int64_t &value);
	static bool GetInt64(KVItemMap &item_map, uint16_t key, uint64_t &value);
	static bool GetBytes(KVItemMap &item_map, uint16_t key, string &str);
	static bool GetBytes(KVItemMap &item_map, uint16_t key, char *&data, uint32_t &size);

public:
	KVData():m_NetTrans(true),m_WriteBuffer(NULL),m_WriteByteBuffer(NULL){}

public:
	//////////////////////////////////////////////////////////////////////
	////////////////////////         打包         ////////////////////////
	//////////////////////////////////////////////////////////////////////
	//往buffer中写入key-value值对.
	//net_trans : true时会调用htons等函数进行转换.
	void AttachWriteBuffer(ByteBuffer *buffer, bool net_trans);
	void AttachWriteBuffer(char *buffer, bool net_trans);

	void SetInt32(uint16_t key, int32_t value);
	void SetInt32(uint16_t key, uint32_t value);
	void SetInt64(uint16_t key, int64_t value);
	void SetInt64(uint16_t key, uint64_t value);
	void SetBytes(uint16_t key, const string &str);
	void SetBytes(uint16_t key, const char *data, uint32_t size);

	//GetDataBuffer和SetDataLength两个方法成对调用,用来写入数据块(当数据量比较大的时候,可减少数据的拷贝次数,提高性能).
	//获取字符串缓冲区,调用者可以直接往缓冲区写数据
	char *GetWriteBuffer(uint16_t key, uint32_t max_size);

	//调用者往缓冲区写入数据后,调用本方法,设置实际写入的数据大小.key和GetWriteBuffer中设置的key必须一致.
	//  如果没调用本方法,则相当于之前调用GetDataBuffer和写入缓冲区的数据无效.
	//  不允许调用本方法之前调用其他SetValue方法.
	void SetWriteLength(uint16_t key, uint32_t size);


	//////////////////////////////////////////////////////////////////////
	////////////////////////         解包         ////////////////////////
	//////////////////////////////////////////////////////////////////////
	bool UnPack(const char *buffer, uint32_t size, bool net_trans);
	//从item-map中获取key对应的value.必需在Unpack成功之后才能调用GetValue方法.
	bool GetInt32(uint16_t key, int32_t &value);
	bool GetInt32(uint16_t key, uint32_t &value);
	bool GetInt64(uint16_t key, int64_t &value);
	bool GetInt64(uint16_t key, uint64_t &value);
	bool GetBytes(uint16_t key, string &str);
	bool GetBytes(uint16_t key, char *&data, uint32_t &size);
private:
	bool m_NetTrans;
	ByteBuffer *m_WriteByteBuffer;
	char *m_WriteBuffer;

	KVItemMap m_ItemMap;
	uint32_t m_Size;
};

inline
void KVData::AttachWriteBuffer(ByteBuffer *buffer, bool net_trans)
{
	m_WriteByteBuffer = buffer;
	m_WriteBuffer = NULL;
	m_NetTrans = net_trans;
}

inline
void KVData::AttachWriteBuffer(char *buffer, bool net_trans)
{
	m_WriteByteBuffer = NULL;
	m_WriteBuffer = buffer;
	m_NetTrans = net_trans;
}

inline
void KVData::SetInt32(uint16_t key, int32_t value)
{
	assert(m_WriteBuffer!=NULL || m_WriteByteBuffer!=NULL);
	if(m_WriteBuffer != NULL)
		SetInt32(key, value, m_WriteBuffer, m_NetTrans);
	else
		SetInt32(key, value, m_WriteByteBuffer, m_NetTrans);
}

inline
void KVData::SetInt32(uint16_t key, uint32_t value)
{
	SetInt32(key, (int32_t)value);
}

inline
void KVData::SetInt64(uint16_t key, int64_t value)
{
	assert(m_WriteBuffer!=NULL || m_WriteByteBuffer!=NULL);
	if(m_WriteBuffer != NULL)
		SetInt64(key, value, m_WriteBuffer, m_NetTrans);
	else
		SetInt64(key, value, m_WriteByteBuffer, m_NetTrans);
}

inline
void KVData::SetInt64(uint16_t key, uint64_t value)
{
	SetInt64(key, (int64_t)value);
}

inline
void KVData::SetBytes(uint16_t key, const string &str)
{
	assert(m_WriteBuffer!=NULL || m_WriteByteBuffer!=NULL);
	if(m_WriteBuffer != NULL)
		SetBytes(key, str, m_WriteBuffer, m_NetTrans);
	else
		SetBytes(key, str, m_WriteByteBuffer, m_NetTrans);
}

inline
void KVData::SetBytes(uint16_t key, const char *data, uint32_t size)
{
	assert(m_WriteBuffer!=NULL || m_WriteByteBuffer!=NULL);
	if(m_WriteBuffer != NULL)
		SetBytes(key, data, size, m_WriteBuffer, m_NetTrans);
	else
		SetBytes(key, data, size, m_WriteByteBuffer, m_NetTrans);
}

inline
char* KVData::GetWriteBuffer(uint16_t key, uint32_t max_size)
{
	assert(m_WriteBuffer!=NULL || m_WriteByteBuffer!=NULL);
	if(m_WriteBuffer != NULL)
		return GetWriteBuffer(key, m_WriteBuffer, m_NetTrans);
	else
		return GetWriteBuffer(key, max_size, m_WriteByteBuffer, m_NetTrans);
}

inline
void KVData::SetWriteLength(uint16_t key, uint32_t size)
{
	assert(m_WriteBuffer!=NULL || m_WriteByteBuffer!=NULL);
	if(m_WriteBuffer != NULL)
		SetWriteLength(key, size, m_WriteBuffer, m_NetTrans);
	else
		SetWriteLength(key, size, m_WriteByteBuffer, m_NetTrans);
}

inline
bool KVData::UnPack(const char *buffer, uint32_t size, bool net_trans)
{
	return UnPack(m_ItemMap, buffer, size, net_trans);
}

inline
bool KVData::GetInt32(uint16_t key, int32_t &value)
{
	return GetInt32(m_ItemMap, key, value);
}

inline
bool KVData::GetInt32(uint16_t key, uint32_t &value)
{
	return GetInt32(m_ItemMap, key, value);
}

inline
bool KVData::GetInt64(uint16_t key, int64_t &value)
{
	return GetInt64(m_ItemMap, key, value);
}

inline
bool KVData::GetInt64(uint16_t key, uint64_t &value)
{
	return GetInt64(m_ItemMap, key, value);
}

inline
bool KVData::GetBytes(uint16_t key, string &str)
{
	return GetBytes(m_ItemMap, key, str);
}

inline
bool KVData::GetBytes(uint16_t key, char *&data, uint32_t &size)
{
	return GetBytes(m_ItemMap, key, data, size);
}

}//nemespace
#endif //_COMMON_KVDATA_H_TEMP


