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
#include "Logger.h"

namespace easynet
{

typedef list<ProtocolContext*> ProtocolList;
typedef map<int32_t, ProtocolList> SendMap;

class IAppInterface
{
public:
	IAppInterface();
	virtual ~IAppInterface();

	//发送协议(添加到发送队列中等待发送),成功返回true,失败返回false.
	virtual bool SendProtocol(int32_t fd, ProtocolContext *context);
	//从队列中获取一个待发送的协议
	virtual ProtocolContext* GetSendProtocol(int32_t fd);
private:
	SendMap       m_SendMap;        //待发送协议的队列map

	IEventServer      *m_EventServer;
	IProtocolFactory  *m_ProtocolFactory;
	TransHandler      *m_TransHandler;
private:
	DECL_LOGGER(logger);
//////////////////////////////////////////////////////////////////
//////////////////////////   接口方法   //////////////////////////
//////////////////////////////////////////////////////////////////
public:
	//处理收到的请求协议
	// @param fd             : 收到协议的socket fd
	// @param context        : 接收到的协议上下文
	// @param detach_context : 被设置为trues时,由应用层控制context的生存期,应用层需要在适当的时候使用IProtocolFactory销毁context.
	virtual bool OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)=0;


	//处理发送协议的事件
	//协议数据完全发送到socket的缓冲区后调本接口
	virtual void OnSendSucc(int32_t fd, ProtocolContext *context)=0;
	//协议数据发送到缓冲区时发生错误后调用本接口
	virtual void OnSendError(int32_t fd, ProtocolContext *context)=0;
	//协议数据超时未完全发送到socket后调用本接口
	virtual void OnSendTimeout(int32_t fd, ProtocolContext *context)=0;


	//socket发生错误调用本接口
	virtual bool OnSocketError(int32_t fd)=0;
	//socket读写空闲发生超时事件后调用本接口
	virtual bool OnSocketTimeout(int32_t fd)=0;

	//获取EventServer的实例
	virtual IEventServer* GetEventServer();
	//获取ProtocolFactory的实例
	virtual IProtocolFactory* GetProtocolFactory();
	//获取传输handler
	virtual IEventHandler* GetTransHandler();

	//获取数据接收的超时时间.
	//从接收到协议的第一个字节开始,在该时间内如果没有收到完整的数据包将发生接收超时事件
	//默认不超时
	virtual int32_t GetRecvTimeout(){return -1;}
};

}//namespace
#endif //_FRAMEWORK_IAPPINTERFACE_H_


