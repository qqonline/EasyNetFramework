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

//二进制协议接口
class IProtocol
{
public:
	int32_t  protocol_type;     //协议类型
	void     *protocol;         //具体的协议

	virtual ~IProtocol(){}

	void Init()
	{
		protocol_type(-1);
		protocol(NULL);
	}

	//销毁自己
	virtual void Destroy()=0;
};

class IProtocolFactory;
class ProtocolContext: public ProtocolInfo, public IProtocol
{
public:
	void Init(IProtocolFactory *protocol_factory)
	{
		ProtocolInfo::Init();
		IProtocol::Init();

		buffer            = NULL;
		buffer_size       = 0;
		cur_data_size     = 0;
		fd                = 0;
		time_out          = -1;
		expire_time       = -1;
		m_ProtocolFactory = protocol_factory;
	}
	void Destroy()
	{
		m_ProtocolFactory->DeleteContext(this);
	}

public:
	char      *buffer;           //存放数据的缓冲区
	uint32_t  buffer_size;       //缓冲区的大小
	uint32_t  cur_data_size;     //当前接收到/已发送的数据大小

	uint32_t  fd;                //接收/发送的socket fd(暂时没有使用)
	int32_t   time_out;          //超时时间(单位:毫秒),应用层设置
	uint64_t  expire_time;       //超时时间点(单位:毫秒),框架设置
	IProtocolFactory *m_ProtocolFactory;
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
    //@param recv_timeout: 接收协议数据超时时间(从收到协议第一个字节开始,在该时间内没有收到完整的数据时发生超时事件), 默认不超时
	IProtocolFactory():m_MemPool(NULL), m_RecvTimeout(-1){}
	IProtocolFactory(int32_t recv_timeout):m_MemPool(NULL){}
	virtual ~IProtocolFactory();

	//创建接收协议的context
	virtual ProtocolContext* NewRecvContext(uint64_t now_time);

	//创建发送协议的context
	// @param max_data_size : 需要发送的数据可能的最大值
	// @param protocol_type : 需要发送的协议类型
	// @param data_type     : 需要发送的数据类型,只能是DTYPE_TEXT或者DTYPE_BIN
	//                          a) 为DTYPE_TEXT时,只分配了max_data_size字节的buffer给应用层写入文本数据,
	//                             应用层需要设置body_size为实际写入的数据大小并且不能超过max_data_size;
	//                          b) 为DTYPE_BIN时,除了生成buffer缓冲区外,还创建了protocol_type类的协议protocol;
	virtual ProtocolContext* NewSendContext(DataType data_type, uint32_t max_data_size, uint32_t protocol_type);

	//删除接收/发送协议的context
	virtual void DeleteContext(ProtocolContext *context);
protected:
	//初始化待接收协议的解码信息
	// data_type=DTYPE_INVALID时,data_header_size必需设置;
	// data_type=DTYPE_TEXT时,body_size必需设置为文本协议数据可能的最大长度;
	// data_type=DTYPE_BIN时,header_size必需设置;
	virtual bool InitRecvInfo(ProtocolInfo *pro_info)=0;

	//检测是二进制还是文本数据
	//成功的话,设置data_type(DTYPE_BIN时必须设置header_size; DTYPE_TEXT时必须设置body_size,表示文本协议数据可能的最大长度)
	virtual DecodeResult DecodeDataType(ProtocolContext *protocol_context)=0;
	//解码二进制协议的协议头,设置body_size.
	virtual DecodeResult DecodeBinHeader(ProtocolContext *protocol_context)=0;
	//解码二进制协议头数据
	virtual DecodeResult DecodeBinBody(ProtocolContext *protocol_context)=0;
	//解码文本协议体数据
	virtual DecodeResult DecodeTextBody(ProtocolContext *protocol_context)=0;

	//创建protocol_type类型的protocol. buffer是大小为buffer_size字节的可用缓冲区(如果创建的protocol需要使用的话)
	virtual void* NewProtocol(uint32_t protocol_type, char *buffer, uint32_t buffer_size)=0;
	//销毁protocol_type类型的protocol
	virtual void DeleteProtocol(void *protocol, uint32_t protocol_type)=0;

	//对协议进行编码,编码数据放到大小为buffer_size字节的buffer中.成功返回true,失败返回false
	//buffer是NewProtocol方法中传入的缓冲区.如果在NewProtocol方法中创建的protocol使用了buffer,
	//则对该protocol进行编码时,应该忽略本buffer参数,否则可能会引起问题!
	virtual bool EncodeProtocol(IProtocol *protocol, char *buffer, uint32_t buffer_size)=0;
protected:
	MemPool *m_MemPool;
	int32_t m_RecvTimeout;
};

}//namespace
#endif //_FRAMEWORK_IPROTOCOL_FACTORY_H_


