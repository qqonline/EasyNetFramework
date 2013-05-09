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
#include "EventServer.h"

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
protected:
	void OnCreateAppInstance();
	void OnDestroyAppInstance();
private:
	SendMap      *m_SendMap;        //待发送协议的队列map
	TransHandler m_TransHandler;

	IEventServer     *m_EventServer;
	IProtocolFactory *m_ProtocolFactory;

	IEventServer* GetEventServer(){return m_EventServer;}
	IProtocolFactory* GetProtocolFactory(){return m_ProtocolFactory;}
	TransHandler* GetTransHandler(){return &m_TransHandler;}

//////////////////////////////////////////////////////////////////
//////////////////////////   接口方法   //////////////////////////
//////////////////////////////////////////////////////////////////
public:
	//创建AppInstance后第一步要做的
	virtual bool StartInstance()=0;
	//销毁AppInstance前最后要做的
	virtual bool StopInstance()=0;

	//处理收到的请求协议.
	// @param fd             : 收到协议的socket fd
	// @param context        : 接收到的协议上下文
	// @param detach_context : 被设置为trues时,由应用层控制context的生存期
	virtual bool OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)=0;
	virtual bool OnSocketError(int32_t fd)=0;
	virtual bool OnSocketTimeout(int32_t fd)=0;

	//处理发送协议的事件
	virtual void OnSendSucc(int32_t fd, ProtocolContext *context)=0;
	virtual void OnSendError(int32_t fd, ProtocolContext *context)=0;
	virtual void OnSendTimeout(int32_t fd, ProtocolContext *context)=0;

protected:
	//派生类必须创建自己的EventServer,ProtocolFactory实例
	IEventServer* CreateEventServer()=0;
	void DestroyEventServer(IEventServer *event_server)=0;

	IProtocolFactory* CreateProtocolFactory()=0;
	void DestroyProtocolFactory(IProtocolFactory *protocol_factory)=0;
};

}//namespace
#endif //_FRAMEWORK_IAPPINTERFACE_H_


