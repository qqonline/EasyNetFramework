/*
 * KVData_example.cpp
 *
 *  Created on: 2013-4-29
 *      Author: LiuYongJin
 */
#include <stdio.h>
#include <string.h>
#include "KVData.h"
using namespace easynet;

#define KEY_INT32    1
#define KEY_INT64    2
#define KEY_STRING_0 3
#define KEY_STRING_1 4

int main()
{
	bool net_trans = true;
	ByteBuffer bytebuffer;

	KVData kv_data;
	kv_data.AttachWriteBuffer(&bytebuffer, net_trans);

	kv_data.SetInt32(KEY_INT32, -26);
	kv_data.SetInt64(KEY_INT64, -36);
	const char *s = "abc";
	kv_data.SetBytes(KEY_STRING_0, s, strlen(s)+1);

	char *temp = kv_data.GetWriteBuffer(KEY_STRING_1, 100);
	strcpy(temp, "XXXXXXXX");
	kv_data.SetWriteLength(KEY_STRING_1, strlen(temp)+1);

	if(!kv_data.UnPack(bytebuffer.Buffer, bytebuffer.Size, net_trans))
	{
		printf("unpack failed.\n");
		return -1;
	}

	int32_t a;
	kv_data.GetInt32(KEY_INT32, a);
	printf("i32=%d\n", a);

	int64_t b;
	kv_data.GetInt64(KEY_INT64, b);
	printf("i64=%lld\n", b);

	char *bytes=NULL;
	uint32_t size;
	kv_data.GetBytes(KEY_STRING_0, bytes, size);
	printf("bytes_0=%s len=%d\n", bytes, size);

	kv_data.GetBytes(KEY_STRING_1, bytes, size);
	printf("bytes_1=%s, len=%d\n", bytes, size);

	return false;
}

