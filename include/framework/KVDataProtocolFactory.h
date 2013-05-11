/*
 * KVDataProtocolFactory.h
 *
 *  Created on: 2013-5-10
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_KVDATA_PROTOCOL_FACTORY_H_
#define _FRAMEWORK_KVDATA_PROTOCOL_FACTORY_H_

#include "IProtocolFactory.h"
#include "Logger.h"

namespace easynet
{

class KVDataProtocolFactory: public IProtocolFactory
{
//////////////////////////////////////////////////////////////////
//////////////////////   派生类实现的接口   //////////////////////
//////////////////////////////////////////////////////////////////
public:
	//检测是二进制还是文本数据
	//成功的话,设置data_type(DTYPE_BIN时必须设置header_size; DTYPE_TEXT时必须设置body_size,表示文本协议数据可能的最大长度)
	//DecodeResult DecodeDataType(ProtocolContext *context);
	//不需要

	//解码二进制协议头数据
	//返回DECODE_SUCC时,buffer,buffer_size,body_size等需要进行设置.
	DecodeResult DecodeBinHeader(ProtocolContext *context);

	//解码二进制协议体数据
	DecodeResult DecodeBinBody(ProtocolContext *context);

	//解码文本协议体数据
	//DecodeResult DecodeTextBody(ProtocolContext *context);
	//不需要

	//对协议进行编码
	virtual bool EncodeProtocol(ProtocolContext *send_context);

	//对协议进行编码
	//编码数据放到大小为buffer_size字节的buffer中.成功返回true,失败返回false
	//buffer是NewProtocol方法中传入的缓冲区.如果在NewProtocol方法中创建的protocol使用了buffer,
	//则对该protocol进行编码时,应该忽略buffer参数,否则可能会引起问题!
	bool EncodeProtocol(void *protocol, int32_t protocol_type, char *buffer, uint32_t buffer_size);

protected:
	//定义协议参数,控制接收协议的类型.
	//初始化待接收协议的解码信息.
	//  data_type=DTYPE_INVALID时,data_header_size必需设置;
	//  data_type=DTYPE_TEXT时,body_size必需设置为文本协议数据可能的最大长度;
	//  data_type=DTYPE_BIN时,header_size必需设置;
	void InitRecvDefine(ProtocolDefine *protocol_def);

	//创建protocol_type类型的protocol.一般在解码协议体成功或者创建send context的时候调用
	//buffer是大小为buffer_size字节的可用缓冲区,如果创建的protocol需要时可以使用该buffer.否则忽略
	//主要用于二进制数据的协议
	void* NewProtocol(int32_t protocol_type, char *buffer, uint32_t buffer_size);

	//销毁protocol_type类型的protocol
	void DeleteProtocol(void *protocol, int32_t protocol_type);

private:
	DECL_LOGGER(logger);

};

}//namespace
#endif //_FRAMEWORK_KVDATA_PROTOCOL_FACTORY_H_


