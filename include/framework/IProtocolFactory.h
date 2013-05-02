/*
 * IProtocolFactory.h
 *
 *  Created on: 2013-4-29
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_PROTOCOL_FACTORY_H_
#define _FRAMEWORK_PROTOCOL_FACTORY_H_

namespace easynet
{
typedef enum _decode_result
{
	DECODE_SUCC,
	DECODE_ERROR,
	DECODE_CONTINUE,
}DecodeResult;

class IProtocolFactory
{
public:
	virtual ~IProtocolFactory(){}
	virtual DecodeResult Decode(char* data, uint32_t len, void** context);
};

}//namespace
#endif //_FRAMEWORK_PROTOCOL_FACTORY_H_


