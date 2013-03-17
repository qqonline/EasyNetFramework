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
	{}

	~SocketWapper(){if(m_msg)delete m_msg;}

	bool attach(Message *msg)
	{
		if(m_msg!=NULL || msg==NULL)
			return false;
		m_msg = msg;
		return msg;
	}

	Message* detach()
	{
		Message *temp = m_msg;
		m_msg = NULL;
		return temp;
	}
	bool has_message(){return m_msg!=NULL;}
private:
	Message m_msg;
};

}//namespace
#endif //_FRAMEWORK_SOCKET_WAPPER_H_
