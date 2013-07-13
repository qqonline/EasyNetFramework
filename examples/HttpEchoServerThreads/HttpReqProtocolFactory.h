/*
 * HttpReqProtocolFactory.h
 *
 *  Created on: 2013-06-24
 *      Author: tim
 */

#ifndef _HTTPREQPROTOCOLFACTORY_H_
#define _HTTPREQPROTOCOLFACTORY_H_

#include "IProtocolFactory.h"
#include "IMemory.h"
#include "HttpParser.h"

using namespace easynet;

class HttpReqProtocolFactory:public IProtocolFactory
{
public:
	HttpReqProtocolFactory(IMemory *memory):m_Memory(memory){}

////////////////////////////////////
////       实现基类接口         ////
////////////////////////////////////
public:
	//能够识别二进制/文本协议的头部大小
	uint32_t HeaderSize();

	//从buffer中解码头部,并获取协议体数据.成功返回true,失败返回false.
	DecodeResult DecodeHeader(const char *buffer, DataType &type, uint32_t &body_size);

	//将头部数据编码写入到buffer.body_size为协议体大小
	void EncodeHeader(char *buffer, uint32_t body_size);

	//解码二进制协议体数据
	virtual DecodeResult DecodeBinBody(ProtocolContext *context);

	//解码文本协议体数据
	DecodeResult DecodeTextBody(ProtocolContext *context);

	//删除DecodeBinBody或DecodeTextBody时创建的protocol实例
	void DeleteProtocol(uint32_t protocol_type, void *protocol);
private:
	IMemory *m_Memory;
};

#endif //_HTTPREQPROTOCOLFACTORY_H_


