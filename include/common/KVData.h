/*
 * KVData.h
 *
 *  Created on: Apr 26, 2013
 *      Author: LiuYongJin
 */
#ifndef _COMMON_KEY_VALUE_DATA_H_
#define _COMMON_KEY_VALUE_DATA_H_

#include <stdint.h>
#include <map>
using std::map;
#include <string>
using std::string;

#include "IMemory.h"

namespace easynet
{

typedef map<uint16_t, void*> PosMap;

//key-value 数据格式.
//key的取值范围:0~4095(即KVData最多只能设置4k个key)
class KVData
{
public:
	//使用
	KVData();
	KVData(IMemory *memory);
	~KVData();

	//返回当前数据大小
	uint32_t Size(){return m_Size;}

	bool Set(uint16_t key, int8_t val);
	bool Set(uint16_t key, int16_t val);
	bool Set(uint16_t key, int32_t val);
	bool Set(uint16_t key, int64_t val);
	bool Set(uint16_t key, const void *bytes, uint32_t size);
	bool Set(uint16_t key, const char *c_str); //c风格的字符串(包括'\0')

	bool UnPack();
	bool Get(uint16_t key, int32_t *val);    //8,16,32都用本方法返回
	bool Get(uint16_t key, int64_t *val);    //64位的用本方法返回
	bool Get(uint16_t key, void **bytes, uint32_t *size);

private:
	uint32_t   m_Size;
	uint32_t   m_Capacity;
	void       *m_Buffer;
	PosMap     m_PosMap;
	bool       m_UseInternalBuffer;

	void _InitMagicNum();
	bool _Set(uint16_t key, uint16_t type, void *bytes, uint32_t size);

	DefaultMemory m_DefaultMemory;
	IMemory *m_Memory;
};

}//namespace
#endif //_COMMON_KEY_VALUE_PROTOCOL_H_



