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

#include "Logger.h"

namespace easynet
{

typedef map<uint16_t, void*> PosMap;

//key-value 数据格式.
//key的取值范围:0~4095(即KVData最多只能设置4k个key)
class KVData
{
public:
	KVData();
	//使用外部buffer. buffer_size必须不小于4
	KVData(void *buffer, uint32_t buffer_size);
	~KVData();

	//返回当前数据大小
	uint32_t Size(){return m_Size;}

	//使用外部buffer. buffer_size必须不小于4
	//当data_size大于0时保持buffer的数据不变,不对buffer进行初始化
	bool AttachBuffer(void *buffer, uint32_t buffer_size, uint32_t data_size=0);
	bool DetachBuffer(void *&buffer, uint32_t &buffer_size, uint32_t &data_size);

	bool Set(uint16_t key, int8_t val);
	bool Set(uint16_t key, uint8_t val);
	bool Set(uint16_t key, int16_t val);
	bool Set(uint16_t key, uint16_t val);
	bool Set(uint16_t key, int32_t val);
	bool Set(uint16_t key, uint32_t val);
	bool Set(uint16_t key, int64_t val);
	bool Set(uint16_t key, uint64_t val);
	bool Set(uint16_t key, const void *bytes, uint32_t size);
	bool Set(uint16_t key, const string &str);

	bool UnPack();
	bool Get(uint16_t key, int8_t *val);
	bool Get(uint16_t key, uint8_t *val);
	bool Get(uint16_t key, int16_t *val);
	bool Get(uint16_t key, uint16_t *val);
	bool Get(uint16_t key, int32_t *val);
	bool Get(uint16_t key, uint32_t *val);
	bool Get(uint16_t key, int64_t *val);
	bool Get(uint16_t key, uint64_t *val);
	bool Get(uint16_t key, void **bytes, uint32_t *size);
	bool Get(uint16_t key, string &str);

private:
	uint32_t   m_Size;
	uint32_t   m_Capacity;
	void       *m_Buffer;
	PosMap     m_PosMap;
	bool       m_UseInternalBuffer;

	bool _Set(uint16_t key, uint16_t type, void *bytes, uint32_t size);
	bool _Get(uint16_t key, uint16_t type, void **bytes, uint32_t *size=NULL);
	bool _ExpandCapacity(uint32_t need_size);
private:
	DECL_LOGGER(logger);
};

}//namespace
#endif //_COMMON_KEY_VALUE_PROTOCOL_H_



