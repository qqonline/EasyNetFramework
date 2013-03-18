/*
 * message.h
 *
 *  Created on: 2013-3-17
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_MESSAGE_H_
#define _FRAMEWORK_MESSAGE_H_

#include <assert.h>
#include <string>
using std::string;

#include <common/bytebuffer.h>

namespace easynet
{

class Message
{
public:
	Message(ByteBuffer *bytebuf, const string &detail, uint64_t timestamp=0)
		:m_bytebuf(bytebuf)
		,m_detail(detail)
		,m_timestamp(timestamp)
	{
		assert(bytebuf != NULL);
	}
	Message(ByteBuffer *bytebuf, const char *detail, uint64_t timestamp=0)
		:m_bytebuf(bytebuf)
		,m_detail(detail)
		,m_timestamp(timestamp)
	{
		assert(bytebuf != NULL);
	}

	~Message(){delete m_bytebuf;}
public:
	ByteBuffer *m_bytebuf;
	string m_detail;
	uint64_t m_timestamp;    //时间戳
};


}//namespace
#endif //_FRAMEWORK_MESSAGE_H_


