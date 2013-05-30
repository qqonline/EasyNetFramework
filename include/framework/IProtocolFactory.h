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

#include "IMemory.h"

namespace easynet
{

//协议数据的类型
typedef enum _query_type
{
	DTYPE_UNKNOW,    //未知协议格式
	DTYPE_TEXT,      //文本数据
	DTYPE_BIN,       //二进制数据
	DTYPE_BIN_TEXT   //文本和二进制
}DataType;

//二进制协议接口
class IProtocol
{
public:
	IProtocol()
		:protocol_type(-1)
		,protocol(NULL)
		,info((char*)"")
	{
	}
	virtual ~IProtocol(){}

	//销毁自己
	virtual void Destroy()=0;

public:
	int32_t  protocol_type;     //协议类型
	void     *protocol;         //具体的协议实例
	char     *info;             //跟协议有关的描述信息
};

class IProtocolFactory;
class ProtocolContext: public IProtocol
{
	friend class IProtocolFactory;
public:
	void Destroy();
public:
	DataType   type;               //数据类型
	uint32_t   expect_size;        //期望的总数据长度
	ByteBuffer *bytebuffer;        //缓冲区
	int32_t    time_out;           //接收/发送的超时时间(单位:毫秒),应用层设置
	uint64_t   expire_time;        //接收/发送的超时时间点(单位:毫秒),框架设置
	uint32_t   fd;                 //接收/发送的socket fd(暂时没有使用)
private:
	ProtocolContext(IProtocolFactory *factory, IMemory *mem);
	~ProtocolContext();
private:
	IMemory   *m_Memory;
	IProtocolFactory *m_Factory;
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
	IProtocolFactory(IMemory *memory);
	virtual ~IProtocolFactory(){}

	//创建接收协议的context
	ProtocolContext* NewRecvContext();

	//创建文本协议的context
	ProtocolContext* NewSendContextText(uint32_t max_size);

	//创建二进制协议的context
	//  @param protocol_type : 需要发送的协议类型
	//  @param max_size : 需要发送的数据可能的最大值
	ProtocolContext* NewSendContextBin(uint32_t protocol_type, uint32_t max_size);

	//删除接收/发送协议的context
	void DeleteContext(ProtocolContext *context);

protected:
	IMemory  *m_Memory;

//////////////////////////////////////////////////////////////////
//////////////////////   派生类实现的接口   //////////////////////
//////////////////////////////////////////////////////////////////
public:
	//初始化RecvContext,设置data_type为:
	//  (1) DTYPE_INVALID: 表明支持二进制和文本格式,但数据所属格式位置,需解码数据格式. header_size必须设置为区分二进制和文本数据的长度.
	//  (2) DTYPE_BIN    : 表明只支持二进制格式,需设置header_size为二进制协议头长度.
	//  (3) DTYPE_TEXT   : 表明只支持文本格式,需设置body_size为文本格式可能的最大数据长度.
	virtual void InitDecodeParameter(DataType &type, uint32_t &expect_size)=0;

	//初始化二进制SendContext的头部长度和最大的协议体长度
	virtual uint32_t GetBinHeaderSize()=0;

	//同时支持文本和二进制数据时必须实现,用于检测是二进制还是文本数据格式.
	//成功的话,设置data_type及相关参数:
	//  (1) data_type=DTYPE_BIN时必须设置header_size表明二进制协议头长度;
	//  (2) data_type=DTYPE_TEXT时必须设置body_size,表示文本协议数据可能的最大长度
	virtual DecodeResult DecodeDataType(ProtocolContext *context)
	{
		assert(0);
		return DECODE_SUCC;
	}

	//解码二进制协议头数据
	//返回DECODE_SUCC时,buffer,buffer_size,body_size等需要进行设置.
	virtual DecodeResult DecodeBinHeader(ProtocolContext *context)
	{
		assert(0);
		return DECODE_SUCC;
	}

	//解码二进制协议体数据
	virtual DecodeResult DecodeBinBody(ProtocolContext *context)
	{
		assert(0);
		return DECODE_SUCC;
	}

	//解码文本协议体数据
	virtual DecodeResult DecodeTextBody(ProtocolContext *context)
	{
		assert(0);
		return DECODE_SUCC;
	}

	//对协议进行编码
	virtual bool EncodeProtocol(ProtocolContext *send_context)=0;

	//对协议进行编码
	//编码数据放到大小为buffer_size字节的buffer中.成功返回true,失败返回false
	//buffer是NewProtocol方法中传入的缓冲区.如果在NewProtocol方法中创建的protocol使用了buffer,
	//则对该protocol进行编码时,应该忽略buffer参数,否则可能会引起问题!
	virtual bool EncodeProtocol(void *protocol, int32_t protocol_type, char *buffer, uint32_t buffer_size)
	{
		assert(0);
		return DECODE_SUCC;
	}

protected:
	//创建protocol_type类型的protocol.一般在解码协议体成功或者创建send context的时候调用
	//buffer是大小为buffer_size字节的可用缓冲区,如果创建的protocol需要时可以使用该buffer.否则忽略
	//主要用于二进制数据的协议
	virtual void* NewProtocol(int32_t protocol_type, ByteBuffer *byte_buffer)
	{
		assert(0);
		return NULL;
	}

	//销毁protocol_type类型的protocol
	virtual void DeleteProtocol(int32_t protocol_type, void *protocol)
	{
		assert(0);
		return ;
	}

};

}//namespace
#endif //_FRAMEWORK_IPROTOCOL_FACTORY_H_


