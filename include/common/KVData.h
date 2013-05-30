/*
 * KVData.h
 *
 *  Created on: 2013-5-24
 *      Author: LiuYongJin
 */

#ifndef _COMMON_KVDATA_H_TEMP
#define _COMMON_KVDATA_H_TEMP

#include <assert.h>
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
		int32_t value_i32;
		int64_t value_i64;
		uint32_t length;
	}value;
	char *value_bytes;
}KVItem;

typedef map<uint16_t, KVItem> KVItemMap;

class KVData
{
public:
	//////////////////////////////////////////////////////////////////////
	////////////////////////         打包         ////////////////////////
	//////////////////////////////////////////////////////////////////////
	//往buffer中写入key-value值对.
	//net_trans : true时会调用htons等函数进行转换.
	static void SetValue(uint16_t key, int32_t value, ByteBuffer *buffer, bool net_trans=true);
	static void SetValue(uint16_t key, int64_t value, ByteBuffer *buffer, bool net_trans=true);
	static void SetValue(uint16_t key, const string &str, ByteBuffer *buffer, bool net_trans=true);
	static void SetValue(uint16_t key, const char *c_str, ByteBuffer *buffer, bool net_trans=true);    //c风格字符串,包含'\0'
	static void SetValue(uint16_t key, const char *data, uint32_t size, ByteBuffer *buffer, bool net_trans=true);

	//GetDataBuffer和SetDataLength两个方法成对调用,用来写入数据块(当数据量比较大的时候,可以减少数据的拷贝次数,提高性能).
	//获取字符串缓冲区,调用者可以直接往缓冲区写数据
	static char *GetWriteBuffer(uint16_t key, uint32_t max_size, ByteBuffer *buffer, bool net_trans=true);

	//调用者往缓冲区写入数据后,调用本方法,设置实际写入的数据大小.
	//  如果没调用本方法,则相当于之前调用GetDataBuffer和写入缓冲区的数据无效.
	//  不允许调用本方法之前调用其他SetValue方法.
	static void SetWriteLength(uint16_t key, uint32_t size, ByteBuffer *buffer, bool net_trans=true);

	//////////////////////////////////////////////////////////////////////
	////////////////////////         解包         ////////////////////////
	//////////////////////////////////////////////////////////////////////
	//对buffer进行解包,获取key-value值对,解包后的key-value保存到item_map中.
	static bool UnPack(KVItemMap &item_map, const char *buffer, uint32_t size, bool net_trans=true);

	//从item-map中获取key对应的value.必需在Unpack成功之后才能调用GetValue方法.
	static bool GetValue(KVItemMap &item_map, uint16_t key, int32_t &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, int64_t &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, string &str);
	static bool GetValue(KVItemMap &item_map, uint16_t key, char *&c_str);
	static bool GetValue(KVItemMap &item_map, uint16_t key, char *&data, uint32_t &size);


public:
	KVData():m_NetTrans(true),m_WriteBuffer(NULL),m_ReadBuffer(NULL),m_ReadSize(0){}

public:
	//////////////////////////////////////////////////////////////////////
	////////////////////////         打包         ////////////////////////
	//////////////////////////////////////////////////////////////////////

	//net_trans : true时会调用htons等函数进行转换.
	void AttachWriteBuffer(ByteBuffer *buffer, bool net_trans=true);

	void SetValue(uint16_t key, int32_t value);
	void SetValue(uint16_t key, int64_t value);
	void SetValue(uint16_t key, const string &str);
	void SetValue(uint16_t key, const char *c_str);    //c风格字符串,包含'\0'
	void SetValue(uint16_t key, const char *data, uint32_t size);

	//GetDataBuffer和SetDataLength两个方法成对调用,用来写入数据块(当数据量比较大的时候,可以减少数据的拷贝次数,提高性能).
	//获取字符串缓冲区,调用者可以直接往缓冲区写数据
	char *GetWriteBuffer(uint16_t key, uint32_t max_size);

	//调用者往缓冲区写入数据后,调用本方法,设置实际写入的数据大小.
	//  如果没调用本方法,则相当于之前调用GetDataBuffer和写入缓冲区的数据无效.
	//  不允许调用本方法之前调用其他SetValue方法.
	void SetWriteLength(uint16_t key, uint32_t size);


	//////////////////////////////////////////////////////////////////////
	////////////////////////         解包         ////////////////////////
	//////////////////////////////////////////////////////////////////////
	void AttachReadBuffer(const char *buffer, uint32_t size, bool net_trans=true);

	//对buffer进行解包,获取key-value值对,解包后的key-value保存到item_map中.
	bool UnPack();

	//从item-map中获取key对应的value.必需在Unpack成功之后才能调用GetValue方法.
	bool GetValue(uint16_t key, int32_t &value);
	bool GetValue(uint16_t key, int64_t &value);
	bool GetValue(uint16_t key, string &str);
	bool GetValue(uint16_t key, char *&c_str);
	bool GetValue(uint16_t key, char *&data, uint32_t &size);
private:
	bool m_NetTrans;
	ByteBuffer *m_WriteBuffer;

	char *m_ReadBuffer;
	uint32_t m_ReadSize;
	KVItemMap m_ItemMap;
};

inline
void KVData::AttachWriteBuffer(ByteBuffer *buffer, bool net_trans/*=true*/)
{
	m_WriteBuffer = buffer;
	m_NetTrans = net_trans;
}

inline
void KVData::SetValue(uint16_t key, int32_t value)
{
	SetValue(key, value, m_WriteBuffer, m_NetTrans);
}
inline
void KVData::SetValue(uint16_t key, int64_t value)
{
	SetValue(key, value, m_WriteBuffer, m_NetTrans);
}

inline
void KVData::SetValue(uint16_t key, const string &str)
{
	SetValue(key, str, m_WriteBuffer, m_NetTrans);
}

inline
void KVData::SetValue(uint16_t key, const char *c_str)
{
	SetValue(key, c_str, m_WriteBuffer, m_NetTrans);
}

inline
void KVData::SetValue(uint16_t key, const char *data, uint32_t size)
{
	SetValue(key, data, size, m_WriteBuffer, m_NetTrans);
}

inline
char* KVData::GetWriteBuffer(uint16_t key, uint32_t max_size)
{
	return GetWriteBuffer(key, max_size, m_WriteBuffer, m_NetTrans);
}

inline
void KVData::SetWriteLength(uint16_t key, uint32_t size)
{
	SetWriteLength(key, size, m_WriteBuffer, m_NetTrans);
}

inline
void KVData::AttachReadBuffer(const char *buffer, uint32_t size, bool net_trans/*=true*/)
{
	m_ReadBuffer = (char*)buffer;
	m_ReadSize = size;
	m_NetTrans = net_trans;
}

inline
bool KVData::UnPack()
{
	return UnPack(m_ItemMap, m_ReadBuffer, m_ReadSize, m_NetTrans);
}

inline
bool KVData::GetValue(uint16_t key, int32_t &value)
{
	return GetValue(m_ItemMap, key, value);
}

inline
bool KVData::GetValue(uint16_t key, int64_t &value)
{
	return GetValue(m_ItemMap, key, value);
}

inline
bool KVData::GetValue(uint16_t key, string &str)
{
	return GetValue(m_ItemMap, key, str);
}

inline
bool KVData::GetValue(uint16_t key, char *&c_str)
{
	return GetValue(m_ItemMap, key, c_str);
}

inline
bool KVData::GetValue(uint16_t key, char *&data, uint32_t &size)
{
	return GetValue(m_ItemMap, key, data, size);
}

}//nemespace
#endif //_COMMON_KVDATA_H_TEMP


