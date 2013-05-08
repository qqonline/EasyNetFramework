/*
 * IAppInterface.h
 *
 *  Created on: 2013-5-4
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_IAPPINTERFACE_H_
#define _FRAMEWORK_IAPPINTERFACE_H_
#include <list>
#include <map>
using std::list;
using std::map;

#include "IProtocolFactory.h"

namespace easynet
{

typedef list<ProtocolContext*> ProtocolList;
typedef map<int32_t, ProtocolList> SendMap;

class IAppInterface
{
public:
	IAppInterface():m_SendMap(NULL){}
	virtual ~IAppInterface();

	//发送协议(添加到发送队列中等待发送),成功返回true,失败返回false.
	virtual bool SendProtocol(int32_t fd, ProtocolContext *context);
	//从队列中获取一个待发送的协议
	virtual ProtocolContext* GetSendProtocol(int32_t fd);

	//处理收到的请求协议.
	// @param fd             : 收到协议的socket fd
	// @param context        : 接收到的协议上下文
	// @param detach_context : 被设置为trues时,由应用层控制context的生存期
	virtual bool OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)=0;
	virtual bool OnSocketError(int32_t fd)=0;
	virtual bool OnSocketTimeout(int32_t fd)=0;


	virtual void OnSendSucc(int32_t fd, ProtocolContext *context)=0;
	virtual void OnSendError(int32_t fd, ProtocolContext *context)=0;
	virtual void OnSendTimeout(int32_t fd, ProtocolContext *context)=0;
private:
	SendMap *m_SendMap;
};

}//namespace
#endif //_FRAMEWORK_IAPPINTERFACE_H_


