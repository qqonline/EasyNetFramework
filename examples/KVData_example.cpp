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
	//kv_data.Set(KEY_CHAR_A, (int8_t)'A');
	kv_data.Set(KEY_INT16_B, (int16_t)-16);
	kv_data.Set(KEY_INT32_C, -26);
	kv_data.Set(KEY_INT64_D, (int64_t)-36);
	const char *s = "abc";
	//kv_data.Set(KEY_BYTES_E, s);

	if(!kv_data.UnPack())
	{
		printf("unpack failed.\n");
		return -1;
	}

	int32_t a;
	//kv_data.Get(KEY_CHAR_A, &a);
	//printf("i8_a=%c\n", a);

	kv_data.Get(KEY_INT16_B, &a);
	printf("i16_b=%d\n", a);

	kv_data.Get(KEY_INT32_C, &a);
	printf("i32_c=%d\n", a);

	int64_t i64_d;
	kv_data.Get(KEY_INT64_D, &i64_d);
	printf("i64_d=%ld\n", i64_d);

	/*
	char *bytes=NULL;
	uint32_t size;
	kv_data.Get(KEY_BYTES_E, (void**)&bytes, &size);
	printf("bytes=%s len=%d\n", bytes, size);
	*/
	return false;
}

