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
typedef map<uint16_t, KVItem> KVItemMap;

typedef struct _kv_buffer_
{
	const char *first;
	char *second;
	uint16_t key;
	bool net_trans;
}KVBuffer;

class KVData
{
public:
	KVData():m_NetTrans(false), m_Size(0){}
	KVData(bool net_trans):m_NetTrans(net_trans), m_Size(0){}

	void Clear();     //清空数据
	uint32_t Size();  //序列化时,保存值对所需要的字节数

	//序列化
	//  将值对序列化到buffer中,buffer的大小不能小于Size()
	//  返回值:返回序列化后数据大小,和Size()的值应该相等
	uint32_t Serialize(char *buffer);

	//设置key-value值对
	void SetValue(uint16_t key, int8_t   value);
	void SetValue(uint16_t key, uint8_t  value);
	void SetValue(uint16_t key, int16_t  value);
	void SetValue(uint16_t key, uint16_t value);
	void SetValue(uint16_t key, int32_t  value);
	void SetValue(uint16_t key, uint32_t value);
	void SetValue(uint16_t key, int64_t  value);
	void SetValue(uint16_t key, uint64_t value);

	//写字符串方法. 注:只是对数据的"引用",在调用Serialize之前释放数据的内存的话会导致不可知后果(如数据错误等)
	void SetValue(uint16_t key, const string &str);
	void SetValue(uint16_t key, const char *c_str);  //包含'\0'
	void SetValue(uint16_t key, const char *data, uint32_t len);

	//反序列化
	//  成功返回true,失败返回false
	bool UnSerialize(const char *buffer, uint32_t size);

	//获取key对应的值
	//  成功返回true,失败返回false
	bool GetValue(uint16_t key, int8_t   &value);
	bool GetValue(uint16_t key, uint8_t  &value);
	bool GetValue(uint16_t key, int16_t  &value);
	bool GetValue(uint16_t key, uint16_t &value);
	bool GetValue(uint16_t key, int32_t  &value);
	bool GetValue(uint16_t key, uint32_t &value);
	bool GetValue(uint16_t key, int64_t  &value);
	bool GetValue(uint16_t key, uint64_t &value);
	bool GetValue(uint16_t key, char *&data, uint32_t &len);  //注:data只是指向源数据buffer的数据,并没有真正拷贝数据,在源buffer失效后不可再用.
	bool GetValue(uint16_t key, string &str);
public:
	////////////////////////////////////////////////////////////////////
	//// 序列化                                                     ////
	////   将key-value值写入buffer中                                ////
	////   net_trans为true时会调用htons等函数进行转换               ////
	////   值对占用的字节数:头部大小和值占用字节数                  ////
	////   调用者需要保存buffer有足够空间                           ////
	////////////////////////////////////////////////////////////////////
	//存放数据需要占用的字节数,用于调用者检查buffer是否足够
	static uint32_t SizeInt(int8_t);
	static uint32_t SizeInt(uint8_t);
	static uint32_t SizeInt(int16_t);
	static uint32_t SizeInt(uint16_t);
	static uint32_t SizeInt(int32_t);
	static uint32_t SizeInt(uint32_t);
	static uint32_t SizeInt(int64_t);
	static uint32_t SizeInt(uint64_t);
	static uint32_t SizeBytes(uint32_t len);  //存放大小为len字节的数据需要占用的字节数

	//设置key-value值对
	//  返回值:值对占用的字节数
	static uint32_t SetValue(char *buffer, uint16_t key, int8_t   value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, uint8_t  value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, int16_t  value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, uint16_t value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, int32_t  value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, uint32_t value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, int64_t  value, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, uint64_t value, bool net_trans=false);

	//写字符串方法
	static uint32_t SetValue(char *buffer, uint16_t key, const string &str, bool net_trans=false);
	static uint32_t SetValue(char *buffer, uint16_t key, const char *c_str, bool net_trans=false);  //包含'\0'
	static uint32_t SetValue(char *buffer, uint16_t key, const char *data, uint32_t len, bool net_trans=false);

	//反序列化
	//  成功返回true,失败返回false
	static bool UnSerialize(KVItemMap &item_map, const char *buffer, uint32_t size, bool net_trans=false);

	//获取key对应的值
	//  成功返回true,失败返回false
	static bool GetValue(KVItemMap &item_map, uint16_t key, int8_t   &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, uint8_t  &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, int16_t  &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, uint16_t &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, int32_t  &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, uint32_t &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, int64_t  &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, uint64_t &value);
	static bool GetValue(KVItemMap &item_map, uint16_t key, char *&data, uint32_t &len);
	static bool GetValue(KVItemMap &item_map, uint16_t key, string &str);
public:  //高级方法
	//以下两个方法与_写字符串方法_等效,只不过是直接使用buffer存数据,而不是从别的数据copy到buffer中
	//在存大数据时能够减少数据copy的次数,从而提高性能.
	//使用前需要确保buffer有足够的空间:
	//  1.使用SizeBytes判断写入len个字节需要的buffer大小
	//  2.实际写入的字节数不能超过len个字节

	//开始写数据
	//  从buffer中获取一个键值为key的缓冲区用于写入数据
	//  返回值:RawBuffer的second为可写数据的缓冲区
	static KVBuffer BeginWrite(char *buffer, uint16_t key, bool net_trans=false);

	//写数据结束
	//  raw_buf:的BeginWrite返回值
	//  len:实际写入的字节数
	//  返回值:总共占用的字节数
	static uint32_t EndWrite(KVBuffer &raw_buf, uint32_t len);
private:
	bool m_NetTrans;
	uint32_t m_Size;
	KVItemMap m_ItemMap;
};

}//nemespace
#endif //_COMMON_KVDATA_H_TEMP


