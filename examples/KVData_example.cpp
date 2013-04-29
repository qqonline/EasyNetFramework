/*
 * KVData_example.cpp
 *
 *  Created on: 2013-4-29
 *      Author: LiuYongJin
 */
#include <stdio.h>
#include <stdint.h>
#include "KVData.h"
using namespace easynet;

#define KEY_CHAR_A     0
#define KEY_INT16_B    1
#define KEY_UINT16_B   2
#define KEY_INT32_C    3
#define KEY_UINT32_C   4
#define KEY_INT64_D    5
#define KEY_UINT64_D   6
#define KEY_BYTES_E    7
#define KEY_STRING_E   8

int main()
{
	KVData kv_data;
	kv_data.Set(KEY_CHAR_A, (int8_t)'A');
	kv_data.Set(KEY_INT16_B, (int16_t)-16);
	kv_data.Set(KEY_UINT16_B, (uint16_t)17);
	kv_data.Set(KEY_INT32_C, (int32_t)-26);
	kv_data.Set(KEY_UINT32_C, (uint32_t)27);
	kv_data.Set(KEY_INT64_D, (int64_t)-36);
	kv_data.Set(KEY_UINT64_D, (uint64_t)37);
	kv_data.Set(KEY_BYTES_E, "abc", 4);
	string str="ABC";
	kv_data.Set(KEY_STRING_E, str);

	if(!kv_data.UnPack())
	{
		printf("unpack failed.\n");
		return -1;
	}

	int8_t a;
	kv_data.Get(KEY_CHAR_A, &a);
	printf("i8_a=%c\n", a);

	int16_t i16_b;
	kv_data.Get(KEY_INT16_B, &i16_b);
	printf("i16_b=%d\n", i16_b);

	uint16_t ui16_b;
	kv_data.Get(KEY_UINT16_B, &ui16_b);
	printf("ui16_b=%d\n", ui16_b);

	int32_t i32_c;
	kv_data.Get(KEY_INT32_C, &i32_c);
	printf("i32_c=%d\n", i32_c);

	uint32_t ui32_c;
	kv_data.Get(KEY_UINT32_C, &ui32_c);
	printf("ui32_c=%d\n", ui32_c);


	int64_t i64_d;
	kv_data.Get(KEY_INT64_D, &i64_d);
	printf("i64_d=%ld\n", i64_d);

	uint64_t ui64_d;
	kv_data.Get(KEY_UINT64_D, &ui64_d);
	printf("ui64_d=%llu\n", ui64_d);

	char *bytes=NULL;
	uint32_t size;
	kv_data.Get(KEY_BYTES_E, (void**)&bytes, &size);
	printf("bytes=%s len=%d\n", bytes, size);

	string str_a;
	kv_data.Get(KEY_STRING_E, str_a);
	printf("string=%s len=%d\n", str_a.c_str(), str_a.size());

	return false;
}

