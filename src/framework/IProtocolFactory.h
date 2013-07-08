/*
 * IProtocolFactory.h
 *
 *  Created on: 2013-5-4
 *      Author: LiuYongJin
 */
#ifndef _FRAMEWORK_IPROTOCOL_FACTORY_H_
#define _FRAMEWORK_IPROTOCOL_FACTORY_H_
#include <stdint.h>
#include <assert.h>
#include <string>
using std::string;

#include "IMemory.h"
#include "ByteBuffer.h"

namespace easynet
{

//协议数据的类型
typedef enum _data_type
{
	DTYPE_INVALID,   //未知协议格式
	DTYPE_TEXT,      //文本数据
	DTYPE_BIN,       //二进制数据
	DTYPE_BIN_TEXT   //文本和二进制
}DataType;

class ProtocolContext:public ByteBuffer
{
public:
	ProtocolContext():ByteBuffer()
	{
		Init();
	}
	ProtocolContext(uint32_t capacity):ByteBuffer(capacity)
	{
		Init();
	}
	ProtocolContext(IMemory *memory):ByteBuffer(memory)
	{
		Init();
	}
	ProtocolContext(uint32_t capacity, IMemory *memory):ByteBuffer(capacity, memory)
	{
		Init();
	}
public:
	DataType   type;               //数据类型        (接收数据时使用)
	uint32_t   header_size;        //头部长度        (接收数据时使用)
	uint32_t   body_size;          //协议体长度      (接收数据时使用)
	uint32_t   protocol_type;      //协议类型        (接收数据时使用)
	void       *protocol;          //具体协议实例    (接收数据时使用)

	uint32_t   send_size;          //已发送数据大小  (发送数据时使用)
	string     Info;               //提示信息        (发送数据时使用)

	int32_t    timeout_ms;         //接收/发送的超时时间(单位:毫秒),应用层设置
	uint64_t   expire_time;        //接收/发送的超时时间点(单位:毫秒),框架设置
	uint32_t   fd;                 //接收/发送的socket fd(暂时没有使用)
private:
	void Init()
	{
		type = DTYPE_INVALID;
		header_size = 0;
		body_size = 0;
		protocol_type = 0;
		protocol = NULL;

		send_size = 0;

		timeout_ms = -1;
		expire_time = 0;
		fd = 0;
	}
};

typedef enum _decode_result
{
	DECODE_SUCC,    //成功
	DECODE_ERROR,   //错误
	DECODE_DATA,    //数据不够
}DecodeResult;

//支持文本协议和二进制协议
//二进制协议:协议头+协议体; 文本协议:协议体(文本数据)
class IProtocolFactory
{
public:
	virtual ~IProtocolFactory(){}

public:
	//////////////////////////////////////////////////////////////////
	//////////////////////   派生类实现的接口   //////////////////////
	//////////////////////////////////////////////////////////////////
	//能够识别二进制/文本协议的头部大小
	virtual uint32_t HeaderSize()=0;

	//从buffer中解码头部,设置协议数据类型并获取协议体数据长度.成功返回true,失败返回false.
	virtual DecodeResult DecodeHeader(const char *buffer, DataType &type, uint32_t &body_size)=0;

	//将头部数据编码写入到buffer.body_size为协议体大小
	virtual void EncodeHeader(char *buffer, uint32_t body_size)=0;

	//解码二进制协议体数据
	virtual DecodeResult DecodeBinBody(ProtocolContext *context)=0;

	//解码文本协议体数据
	virtual DecodeResult DecodeTextBody(ProtocolContext *context)=0;

	//删除DecodeBinBody或DecodeTextBody时创建的protocol实例
	virtual void DeleteProtocol(uint32_t protocol_type, void *protocol)=0;
};

}//namespace
#endif //_FRAMEWORK_IPROTOCOL_FACTORY_H_


