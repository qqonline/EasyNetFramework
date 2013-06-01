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
#include "TransHandler.h"
#include "ListenHandler.h"
#include "Logger.h"
#include "IMemory.h"

namespace easynet
{

typedef list<ProtocolContext*> ProtocolList;
typedef map<int32_t, ProtocolList> SendMap;

//默认使用以下组件实例:
//    EventServer     : EventServerEpoll
//    ProtocolFactory : KVDataProtocolFactory
//    TransHandler    : TransHandler
//    ListenHandler   : ListenHandler
//    IMemory         : SystemMemory
class IAppInterface
{
public:
	IAppInterface();
	virtual ~IAppInterface();

	//监听端口
	//  @param port     : 需要监听的端口
	//  @param ip       : 需要监听的本地ip(如果没有指定本地的网卡地址,使用默认的)
	//  @param back_log : accept的队列大小
	virtual bool Listen(int32_t port, const char *ip=NULL, uint32_t back_log=128);

	//发送协议(添加到发送队列中等待发送),成功返回true,失败返回false.
	//  @param send_timeout : 发送的超时时间(单位毫秒).在该时间内如果没有发送完成,将产生超时事件,OnSendTimeout接口被调用.默认-1表示不进行超时检查
	virtual bool SendProtocol(int32_t fd, ProtocolContext *context, int32_t send_timeout_ms=-1);

	//从队列中获取一个待发送的协议
	virtual ProtocolContext* GetSendProtocol(int32_t fd);

	//创建协议上下文
	virtual ProtocolContext* NewProtocolContext();

	//删除协议上下文
	virtual void DeleteProtocolContext(ProtocolContext *context);

	//接收一个新的连接,添加到EventServer中,开始接收/发送数据
	virtual bool AcceptNewConnect(int32_t fd);

	//获取EventServer的实例
	virtual IEventServer* GetEventServer();

	//获取ProtocolFactory的实例
	virtual IProtocolFactory* GetProtocolFactory();

	//获取传输handler实例
	virtual IEventHandler* GetTransHandler();

	//获取服务监听handler实例
	virtual IEventHandler* GetListenHander();

	//内存分配器实例
	virtual IMemory* GetMemory();

private:
	SendMap           m_SendMap;        //待发送协议的队列map
	SystemMemory      m_SysMemory;

	IEventServer      *m_EventServer;
	IProtocolFactory  *m_ProtocolFactory;
	TransHandler      *m_TransHandler;
	ListenHandler     *m_ListenHandler;
private:
	DECL_LOGGER(logger);
//////////////////////////////////////////////////////////////////
//////////////////////////   接口方法   //////////////////////////
//////////////////////////////////////////////////////////////////
public:
	//启动App实例
	virtual bool Start()=0;

	//处理收到的请求协议
	//  @param fd             : 收到协议的socket fd
	//  @param context        : 接收到的协议上下文
	//  @param detach_context : 被设置为trues时,由应用层控制context的生存期
	//                          应用层需要在适当的时候调用DeleteProtocolContext释放context实例;
	virtual bool OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)=0;

	//处理发送协议的事件.协议数据完全发送到socket的缓冲区后调本接口
	virtual void OnSendSucc(int32_t fd, ProtocolContext *context)=0;

	//协议数据发送到缓冲区时发生错误后调用本接口
	virtual void OnSendError(int32_t fd, ProtocolContext *context)=0;

	//协议数据超时未完全发送到socket后调用本接口
	virtual void OnSendTimeout(int32_t fd, ProtocolContext *context)=0;

	//socket发生错误调用本接口
	virtual bool OnSocketError(int32_t fd)=0;

	//socket读写空闲发生超时事件后调用本接口
	virtual bool OnSocketTimeout(int32_t fd)=0;

	//获取数据接收的超时时间(单位毫秒).从接收到协议的第一个字节开始,在该时间内如果没有收到完整的数据包将发生接收超时事件.
	virtual int32_t GetRecvTimeoutMS()=0;

	//获取连接空闲超时时间(单位毫秒秒).当连接在该时间内无任何读写事件发生的话,将发生超时事件.
	virtual int32_t GetIdleTimeoutMS()=0;
};

}//namespace
#endif //_FRAMEWORK_IAPPINTERFACE_H_


