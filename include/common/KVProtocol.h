/*
 * KVProtocol.h
 *
 *  Created on: Apr 26, 2013
 *      Author: LiuYongJin
 */
#ifndef _COMMON_KEY_VALUE_PROTOCOL_H_
#define _COMMON_KEY_VALUE_PROTOCOL_H_

#include <stdint.h>
#include <map>
using std::map;
#include <string>
using std::string;

#include "Logger.h"

namespace easynet
{

typedef map<uint16_t, uint8_t*> PosMap;
class KVProtocol
{
public:
	KVProtocol();
	~KVProtocol();
	bool Set(uint16_t key, int8_t val);
	bool Set(uint16_t key, int16_t val);
	bool Set(uint16_t key, int32_t val);
	bool Set(uint16_t key, int64_t val);
	bool Set(uint16_t key, int8_t *bytes, uint32_t size);
	bool Set(uint16_t key, string str);

	bool UnPack();
	bool Get(uint16_t key, int32_t *val);
	bool Get(uint16_t key, int64_t *val);
	bool Get(uint16_t key, int8_t **bytes, uint32_t *size);
	bool Get(uint16_t key, string *str);

private:
	uint32_t   m_Size;
	uint32_t   m_Capacity;
	int8_t    *m_Buffer;
	PosMap     m_PosMap;

	bool _Set(uint16_t key, uint16_t type, int8_t *bytes, uint32_t size);
	bool _Get(uint16_t key, uint16_t type, int8_t **bytes, uint32_t *size);
	bool _ExpandCapacity(uint32_t need_size);
private:
	DECL_LOGGER(logger);
};

}//namespace
#endif //_COMMON_KEY_VALUE_PROTOCOL_H_



