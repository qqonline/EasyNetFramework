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
#include "KVData.h"

namespace easynet
{

class KVDataProtocolFactory: public IProtocolFactory
{
public:
	KVDataProtocolFactory(IMemory *memory=NULL):m_Memory(memory){}
public:
	//////////////////////////////////////////////////////////////////
	//////////////////////   派生类实现的接口   //////////////////////
	//////////////////////////////////////////////////////////////////
	//头部大小
	uint32_t HeaderSize();

	//从buffer中解码头部,并获取协议体数据.成功返回true,失败返回false.
	DecodeResult DecodeHeader(const char *buffer, DataType &type, uint32_t &body_size);

	//将头部数据编码写入到buffer.body_size为协议体大小
	void EncodeHeader(char *buffer, uint32_t body_size);

	//解码二进制协议体数据
	DecodeResult DecodeBinBody(ProtocolContext *context);

	//解码文本协议体数据
	DecodeResult DecodeTextBody(ProtocolContext *context);

	//删除DecodeBinBody时创建的protocol实例
	void DeleteProtocol(uint32_t protocol_type, void *protocol);
private:
	IMemory *m_Memory;
private:
	DECL_LOGGER(logger);
};

}//namespace
#endif //_FRAMEWORK_KVDATA_PROTOCOL_FACTORY_H_


