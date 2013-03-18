/*
 * socketwapper.h
 *
 *  Created on: 2013-3-17
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_SOCKET_WAPPER_H_
#define _FRAMEWORK_SOCKET_WAPPER_H_

#include <common/socket_transceiver.h>
#include <framework/message.h>

namespace easynet
{

class SocketWapper:public SocketTrans
{
public:
	SocketWapper(bool active)
		:SocketTrans(active)
		,m_msg(NULL)
		,m_bytebuf(NULL)
		,m_wait_size(0)
	{}

	~SocketWapper()
	{
		if(m_msg)
			delete m_msg;
		if(m_bytebuf)
			delete m_bytebuf;
	}

	bool attach_output_msg(Message *msg)
	{
		if(m_msg!=NULL || msg==NULL)
			return false;
		m_msg = msg;
		return true;
	}

	Message* detach_output_msg()
	{
		Message *temp = m_msg;
		m_msg = NULL;
		return temp;
	}

	Message* get_output_message()
	{
		return m_msg;
	}

	bool attach_input_buffer(ByteBuffer *bytebuf, uint32_t wait_size)
	{
		if(m_bytebuf!=NULL || bytebuf==NULL || wait_size==0)
			return false;
		m_bytebuf = bytebuf;
		m_wait_size = wait_size;
		return true;
	}
	ByteBuffer* detach_input_buffer()
	{
		ByteBuffer *temp = m_bytebuf;
		m_bytebuf = NULL;
		m_wait_size = 0;
		return temp;
	}
	ByteBuffer* get_input_buffer()
	{
		return m_bytebuf;
	}
	uint32_t get_input_wait_size()
	{
		return m_wait_size;
	}
	void set_input_wait_size(uint32_t wait_size)
	{
		if(m_bytebuf != NULL)
			m_wait_size = wait_size;
	}

private:
	Message *m_msg;           //待发送的msg
	ByteBuffer *m_bytebuf;    //已经接收到数据的buffer
	uint32_t m_wait_size;     //剩下待接收的字节数
};

}//namespace
#endif //_FRAMEWORK_SOCKET_WAPPER_H_
