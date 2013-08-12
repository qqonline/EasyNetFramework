/*
 * KVData_example.cpp
 *
 *  Created on: 2013-4-29
 *      Author: LiuYongJin
 */
#include <stdio.h>
#include <string.h>
#include "KVData.h"
#include "ByteBuffer.h"

using namespace easynet;

#define KEY_INT32    1
#define KEY_INT64    2
#define KEY_STRING_0 3
#define KEY_STRING_1 4
#define KEY_INT8     5
#define KEY_INT16    6

int main()
{
	bool net_trans = true;
	ByteBuffer bytebuffer;

	KVData kv_data;
	kv_data.Clear();

	const char *s = "abc";
	//设置值对
	kv_data.SetValue(KEY_INT8, (int8_t)-8);
	kv_data.SetValue(KEY_INT16, (int16_t)-16);
	kv_data.SetValue(KEY_INT32, (int32_t)-32);
	kv_data.SetValue(KEY_INT64, (int64_t)-64);
	kv_data.SetValue(KEY_STRING_0, s);

	//序列化
	bytebuffer.CheckSize(kv_data.Size());
	kv_data.Serialize(bytebuffer.Buffer);
	bytebuffer.Size = kv_data.Size();

	//直接使用缓冲区
	uint32_t len = kv_data.SizeBytes(100);
	bytebuffer.CheckSize(len);

	char *buffer = bytebuffer.Buffer+bytebuffer.Size;
	KVBuffer kv_buf = kv_data.BeginWrite(buffer, KEY_STRING_1);
	strcpy(kv_buf.second, "XXXXXXXX");
	bytebuffer.Size += kv_data.EndWrite(kv_buf, strlen(kv_buf.second)+1);


	//反序列化
	if(!kv_data.UnSerialize(bytebuffer.Buffer, bytebuffer.Size))
	{
		printf("UnSerialize failed.\n");
		return -1;
	}

	int8_t a8;
	if(kv_data.GetValue(KEY_INT8, a8))
		printf("i8=%d\n", a8);
	int16_t a16;
	if(kv_data.GetValue(KEY_INT16, a16))
		printf("i16=%d\n", a16);

	int32_t a32;
	if(kv_data.GetValue(KEY_INT32, a32))
		printf("i32=%d\n", a32);

	int64_t a64;
	if(kv_data.GetValue(KEY_INT64, a64))
		printf("i64=%lld\n", a64);

	string str;
	uint32_t size;
	if(kv_data.GetValue(KEY_STRING_0, str))
		printf("bytes_0=%s\n", str.c_str());

	char *bytes = NULL;
	if(kv_data.GetValue(KEY_STRING_1, bytes, size))
		printf("bytes_1=%s, len=%d\n", bytes, size);

	return false;
}

