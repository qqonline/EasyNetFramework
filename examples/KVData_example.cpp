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

#define KEY_INT32    1
#define KEY_INT64    2
#define KEY_STRING   3

int main()
{
	bool net_trans = true;
	ByteBuffer bytebuffer;

	KVData kv_data;
	kv_data.AttachWriteBuffer(&bytebuffer, net_trans);

	kv_data.SetValue(KEY_INT32, -26);
	kv_data.SetValue(KEY_INT64, (int64_t)-36);
	const char *s = "abc";
	kv_data.SetValue(KEY_STRING, s);

	kv_data.AttachReadBuffer(bytebuffer.m_Buffer, bytebuffer.m_Size, net_trans);
	if(!kv_data.UnPack())
	{
		printf("unpack failed.\n");
		return -1;
	}

	int32_t a;
	kv_data.GetValue(KEY_INT32, a);
	printf("i32=%d\n", a);

	int64_t b;
	kv_data.GetValue(KEY_INT64, b);
	printf("i64=%lld\n", b);

	char *bytes=NULL;
	uint32_t size;
	kv_data.GetValue(KEY_STRING, bytes, size);
	printf("bytes=%s len=%d\n", bytes, size);

	return false;
}

