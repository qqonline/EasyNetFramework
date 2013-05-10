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
private:
	DECL_LOGGER(logger);

//////////////////////////////////////////////////////////////////
//////////////////////   派生类实现的接口   //////////////////////
//////////////////////////////////////////////////////////////////
protected:
	//初始化待接收协议的解码信息
	// data_type=DTYPE_INVALID时,data_header_size必需设置;
	// data_type=DTYPE_TEXT时,body_size必需设置为文本协议数据可能的最大长度;
	// data_type=DTYPE_BIN时,header_size必需设置;
	bool InitRecvInfo(ProtocolInfo *pro_info);

	//检测是二进制还是文本数据
	//成功的话,设置data_type(DTYPE_BIN时必须设置header_size; DTYPE_TEXT时必须设置body_size,表示文本协议数据可能的最大长度)
	DecodeResult DecodeDataType(ProtocolContext *context);
	//解码二进制协议的协议头,设置body_size.
	DecodeResult DecodeBinHeader(ProtocolContext *context);
	//解码二进制协议头数据
	DecodeResult DecodeBinBody(ProtocolContext *context);
	//解码文本协议体数据
	DecodeResult DecodeTextBody(ProtocolContext *context);

	//创建protocol_type类型的protocol. buffer是大小为buffer_size字节的可用缓冲区(如果创建的protocol需要使用的话)
	void* NewProtocol(int32_t protocol_type, char *buffer, uint32_t buffer_size);
	//销毁protocol_type类型的protocol
	void DeleteProtocol(void *protocol, int32_t protocol_type);

	//对协议进行编码,编码数据放到大小为buffer_size字节的buffer中.成功返回true,失败返回false
	//buffer是NewProtocol方法中传入的缓冲区.如果在NewProtocol方法中创建的protocol使用了buffer,
	//则对该protocol进行编码时,应该忽略buffer参数,否则可能会引起问题!
	bool EncodeProtocol(void *protocol, int32_t protocol_type, char *buffer, uint32_t buffer_size);
};

}//namespace
#endif //_FRAMEWORK_KVDATA_PROTOCOL_FACTORY_H_


