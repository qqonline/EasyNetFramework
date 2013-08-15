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
#include "MessageHandler.h"
#include "Logger.h"
#include "IMemory.h"

namespace easynet
{

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


///////////////////////////////////////
////        框架使用的组件实例        ////
///////////////////////////////////////
	//获取EventServer的实例
	virtual IEventServer* GetEventServer();
	//获取ProtocolFactory的实例
	virtual IProtocolFactory* GetProtocolFactory();
	//获取传输handler实例
	virtual IEventHandler* GetTransHandler();
	//获取服务监听handler实例
	virtual IEventHandler* GetListenHander();
	//获取通知消息handler实例
	virtual IEventHandler* GetMessageHandler();
	//内存分配器实例
	virtual IMemory* GetMemory();


///////////////////////////////////////
////        框架提供的默认方法        ////
///////////////////////////////////////
	//监听端口
	//  @param port     : 需要监听的端口
	//  @param ip       : 需要监听的本地ip(如果没有指定本地的网卡地址,使用默认的)
	//  @param back_log : accept的队列大小
	//  @return         : 成功返回fd,失败返回-1.
	virtual int32_t Listen(int32_t port, const char *ip=NULL, uint32_t back_log=128);
	//监听的socket错误处理: 关闭监听的socket
	virtual void OnListenError(int32_t fd);

	//监听消息,接收别的线程分配过来通知
	virtual bool ListenMessage();
	//向消息管道发送通知
	virtual bool SendMessage(int32_t msg);
	//处理消息通知
	virtual bool OnRecvMessage(int32_t msg);


	//接收一个新的连接,添加到EventServer中,开始接收/发送数据
	//  @param fd : 新接收到的链接socket.
	//  @return   : 成功返回true,失败返回false.
	virtual bool AcceptNewConnect(int32_t fd);

	//创建协议上下文(从GetMemory获得的memory中分配内存空间)
	virtual ProtocolContext* NewProtocolContext();
	//删除协议上下文(删除由NewProtocolContext创建的协议上下文,由GetMemory获得的memory回收内存空间)
	virtual void DeleteProtocolContext(ProtocolContext *context);

	//发送协议(添加context到发送队列中等待发送)
	//  @param fd           : 需要发送数据的socket
	//  @param conetxt      : 待发送的协议上下文
	//  @param send_timeout : 发送的超时时间(单位毫秒).默认-1表示不进行超时检查
	//  @return             : 添加成功返回true,失败返回false(调用者需要处理context).
	//  注: 当添加成功后:
	//      超时未发送完成OnSendTimeout将被调用(处理context);
	//      数据发送成功后OnSendSucc将被调用(处理context);
	//      数据发送失败后OnSendError将被调用(处理context);
	virtual bool SendProtocol(int32_t fd, ProtocolContext *context, int32_t send_timeout_ms=-1);
	//从队列中获取一个待发送的协议
	//  @param fd : 需要发送数据的socket
	//  @return   : 需要被发送的协议上下文,返回NULL时表示无数据需要发送
	virtual ProtocolContext* GetSendProtocol(int32_t fd);

	//通知框架结束socket fd
	virtual bool NotifySocketFinish(int32_t fd);
private:
	typedef list<ProtocolContext*> ProtocolList;
	typedef map<int32_t, ProtocolList> SendMap;
	SendMap           m_SendMap;        //待发送协议的队列map
	SystemMemory      m_SysMemory;

	IEventServer      *m_EventServer;
	IProtocolFactory  *m_ProtocolFactory;
	TransHandler      *m_TransHandler;
	ListenHandler     *m_ListenHandler;
	MessageHandler     *m_MessageHandler;
	int32_t           m_RecvFd;
	int32_t           m_WriteFd;
private:
	DECL_LOGGER(logger);
//////////////////////////////////////////////////////////////////
//////////////////////////   接口方法   //////////////////////////
//////////////////////////////////////////////////////////////////
public:
	//启动App实例
	virtual bool Start()=0;

	//获取数据接收的超时时间(单位毫秒).从数据开始接收到收到完整的数据包所允许的时间.
	virtual int32_t GetSocketRecvTimeout()=0;

	//获取连接空闲的超时时间(单位毫秒秒).当连接在该时间内无任何读写事件发生的话,将发生超时事件.
	virtual int32_t GetSocketIdleTimeout()=0;

	//获取允许的最大链接数.
	virtual int32_t GetMaxConnections()=0;


	//处理收到的请求协议
	//  @param fd             : 收到协议的socket
	//  @param context        : 接收到的协议上下文
	//  @param detach_context : 被设置为trues时,由应用层控制context的生存期
	//                          应用层需要在适当的时候调用DeleteProtocolContext释放context实例;
	virtual bool OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context)=0;

	//处理发送协议的事件.协议数据完全发送到socket的缓冲区后调本接口
	//  @param fd             : 发送数据的socket
	//  @param context        : 发送成功的数据,应用层需要根据创建方式对齐进行释放
	virtual void OnSendSucc(int32_t fd, ProtocolContext *context)=0;

	//协议数据发送到缓冲区时发生错误后调用本接口
	//  @param fd             : 发送数据的socket
	//  @param context        : 发送失败的数据,应用层需要根据创建方式对齐进行释放
	virtual void OnSendError(int32_t fd, ProtocolContext *context)=0;

	//协议数据超时未完全发送到socket后调用本接口
	//  @param fd             : 发送数据的socket
	//  @param context        : 发送超时的数据,应用层需要根据创建方式对齐进行释放
	virtual void OnSendTimeout(int32_t fd, ProtocolContext *context)=0;

	//socket需要结束时调用本接口
	//  @param fd             : 需要结束的socket
	virtual void OnSocketFinished(int32_t fd)=0;
};


}//namespace
#endif //_FRAMEWORK_IAPPINTERFACE_H_


