/*
 * IProtocolFactory.h
 *
 *  Created on: 2013-5-4
 *      Author: LiuYongJin
 */
#ifndef _FRAMEWORK_IPROTOCOL_FACTORY_H_
#define _FRAMEWORK_IPROTOCOL_FACTORY_H_
#include <stdint.h>

#include "ByteBuffer.h"

namespace easynet
{

typedef enum _query_type
{
	DTYPE_INVALID,
	DTYPE_TEXT,  //文本数据
	DTYPE_BIN    //二进制数据
}DataType;

class ProtocolContext
{
public:
	ProtocolContext()
	{
		Init();
	}

	void Init()
	{
		fd = -1;
		data_type = DTYPE_INVALID;
		header_size = 0;
		body_size = 0;
		byte_buffer.Clear();

		protocol = NULL;
		protocol_type = -1;
	}
public:
	DataType data_type;
	int32_t fd;
	uint32_t header_size;    //协议头长度,当data_type=DTYPE_BIN有效
	uint32_t body_size;
	ByteBuffer byte_buffer;

	//具体的协议数据
	void *protocol;
	int32_t protocol_type;
};

typedef enum _decode_result
{
	DECODE_SUCC,    //成功
	DECODE_ERROR,   //错误
	DECODE_DATA,    //数据不够
}DecodeResult;

class SendContext
{
public:
	virtual ~SendContext(){}

	void Init()
	{
		fd = -1;
		expire_time = 0;
		protocol_context = NULL;
	}
public:
	int32_t    fd;
	uint64_t   expire_time;    //超时时间点
	ProtocolContext *protocol_context;     //待发送的协议
};

//支持文本协议和二进制协议
//二进制协议:协议头+协议体; 文本协议:协议体(文本数据)
class IProtocolFactory
{
public:
	virtual ~IProtocolFactory(){}

	virtual ProtocolContext* NewContext()=0;
	virtual void DeleteContext(ProtocolContext *protocol_context)=0;

	//伪协议头长度.用来区分二进制和文本协议
	virtual uint32_t HeaderSize()=0;

	//检测是二进制还是文本数据(4个字节)
	// @return : false失败;true成功,设置data_type:
	//              如果为DTYPE_BIN,设置header_size;
	//              如果为DTYPE_TEXT,设置body_size,表示文本协议数据可能的最大长度.
	virtual DecodeResult DecodeDataType(ProtocolContext *protocol_context)=0;

	//解码二进制协议的协议头,设置body_size.
	virtual DecodeResult DecodeBinHeader(ProtocolContext *protocol_context)=0;

	//解码二进制协议头数据
	virtual DecodeResult DecodeBinBody(ProtocolContext *protocol_context)=0;
	//解码文本协议体数据
	virtual DecodeResult DecodeTextBody(ProtocolContext *protocol_context)=0;

	virtual void* NewProtocol(uint32_t protocol_type)=0;
	virtual void DeleteProtocol(void *protocol, uint32_t protocol_type)=0;
};

}//namespace
#endif //_FRAMEWORK_IPROTOCOL_FACTORY_H_


