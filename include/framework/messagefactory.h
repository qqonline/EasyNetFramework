/*
 * messagefactory.h
 *
 *  Created on: 2013-3-18
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_MESSAGE_FACTORY_H_
#define _FRAMEWORK_MESSAGE_FACTORY_H_

#include <stdint.h>
#include <framework/message.h>

namespace easynet
{

class MsgFactory
{
public:
	MsgFactory();
	virtual ~MsgFactory(){};

	//遇到一个新的消息时,第一次需要读取的消息字节数len
	virtual uint32_t msg_decode_length()=0;

	//解码读进的数据,返回值:
	//  小于0:失败,msg无效;
	//  0:解码完成,msg返回解码后的消息;
	//  大于0:还需要读取的字节数,msg无效;
	virtual int32_t msg_decode(ByteBuffer *bytebuf, Message **msg)=0;
};

}//namespace
#endif //_FRAMEWORK_MESSAGE_FACTORY_H_


