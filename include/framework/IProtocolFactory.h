/*
 * IProtocolFactory.h
 *
 *  Created on: 2013-5-4
 *      Author: LiuYongJin
 */
#ifndef _FRAMEWORK_IPROTOCOL_FACTORY_H_
#define _FRAMEWORK_IPROTOCOL_FACTORY_H_
#include <stdint.h>

#include "MemoryPool.h"

namespace easynet
{

typedef enum _query_type
{
	DTYPE_INVALID,
	DTYPE_TEXT,  //文本数据
	DTYPE_BIN    //二进制数据
}DataType;

class ProtocolInfo
{
public:
	void Init()
	{
		data_type         = DTYPE_INVALID;
		data_header_size  = 0;
		header_size       = 0;
		body_size         = 0;
	}

public:
	DataType  data_type;
	uint32_t  data_header_size;  //用于区分二进制/文本协议的数据头长度,当data_type为DTYPE_INVALID有效
	uint32_t  header_size;       //协议头长度,当data_type=DTYPE_BIN有效
	uint32_t  body_size;         //协议体长度.当data_type=DTYPE_BIN时为固定长度,data_type=DTYPE_TEXT时为可能的最大长度
};

class RecvContext: public ProtocolInfo
{
public:
	void Init()
	{
		ProtocolInfo::Init();
		cur_header_size  = 0;
		cur_body_size    = 0;
		buffer           = NULL;
		buffer_size      = 0;
		cur_data_size    = 0;
		protocol         = NULL;
		protocol_type    = 0;
		fd               = 0;
	}

public:
	uint32_t  cur_header_size;   //当前协议头数据长度
	uint32_t  cur_body_size;     //当前协议体长度

	char      *buffer;           //存放数据的缓冲区
	uint32_t  buffer_size;       //缓冲区的大小
	uint32_t  cur_data_size;     //当前数据大小

	void      *protocol;         //具体的协议
	uint32_t  protocol_type;     //协议类型

	uint32_t  fd;
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
	int32_t     fd;
	uint64_t    expire_time;    //超时时间点
	RecvContext *protocol_context;     //待发送的协议
};

//支持文本协议和二进制协议
//二进制协议:协议头+协议体; 文本协议:协议体(文本数据)
class IProtocolFactory
{
public:
	IProtocolFactory();
	virtual ~IProtocolFactory();

	virtual RecvContext* NewContext();
	virtual void DeleteContext(RecvContext *context);

	//设置协议信息
	// data_type=DTYPE_INVALID时,data_header_size必需设置;
	// data_type=DTYPE_TEXT时,body_size必需设置;
	// data_type=DTYPE_BIN时,header_size必需设置;
	virtual bool InitProtocolInfo(ProtocolInfo *pro_info)=0;

	//伪协议头长度.用来区分二进制和文本协议
	virtual uint32_t DataHeaderSize()=0;

	//检测是二进制还是文本数据
	//成功的话,设置data_type(DTYPE_BIN时必须设置header_size; DTYPE_TEXT时必须设置body_size,表示文本协议数据可能的最大长度)
	virtual DecodeResult DecodeDataType(RecvContext *protocol_context)=0;

	//解码二进制协议的协议头,设置body_size.
	virtual DecodeResult DecodeBinHeader(RecvContext *protocol_context)=0;

	//解码二进制协议头数据
	virtual DecodeResult DecodeBinBody(RecvContext *protocol_context)=0;
	//解码文本协议体数据
	virtual DecodeResult DecodeTextBody(RecvContext *protocol_context)=0;

	virtual void* NewProtocol(uint32_t protocol_type)=0;
	virtual void DeleteProtocol(void *protocol, uint32_t protocol_type)=0;
private:
	MemPool *m_MemPool;
};

}//namespace
#endif //_FRAMEWORK_IPROTOCOL_FACTORY_H_


